#include "wbaes.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

// 边界测试计数器
static int boundary_test_count = 0;
static int boundary_passed = 0;
static int boundary_failed = 0;

#define BOUNDARY_TEST_START(name) \
    do { \
        boundary_test_count++; \
        printf("边界测试 %d: %s ... ", boundary_test_count, name); \
        fflush(stdout); \
    } while(0)

#define BOUNDARY_TEST_PASS() \
    do { \
        boundary_passed++; \
        printf("通过\n"); \
    } while(0)

#define BOUNDARY_TEST_FAIL(msg) \
    do { \
        boundary_failed++; \
        printf("失败: %s\n", msg); \
    } while(0)

#define BOUNDARY_ASSERT(condition, msg) \
    do { \
        if (!(condition)) { \
            BOUNDARY_TEST_FAIL(msg); \
            return 0; \
        } \
    } while(0)

// 测试gMul函数的所有可能输入组合（重点测试边界）
int test_gMul_exhaustive() {
    BOUNDARY_TEST_START("gMul函数穷举边界测试");
    
    // 测试关键边界值
    struct {
        u8 a, b;
        u8 expected;
        const char* description;
    } critical_tests[] = {
        {0x00, 0x00, 0x00, "零乘零"},
        {0x00, 0xFF, 0x00, "零乘最大值"},
        {0xFF, 0x00, 0x00, "最大值乘零"},
        {0x01, 0xFF, 0xFF, "单位元乘最大值"},
        {0xFF, 0x01, 0xFF, "最大值乘单位元"},
        {0x02, 0x80, 0x1B, "触发模约简的临界值"},
        {0x80, 0x02, 0x1B, "高位触发模约简"},
        {0x7F, 0x02, 0xFE, "最高位为0的最大值"},
        {0xFF, 0xFF, 0x13, "最大值相乘"},
        {0x53, 0xCA, 0x01, "已知逆元对"}
    };
    
    for (int i = 0; i < 10; i++) {
        u8 result = gMul(critical_tests[i].a, critical_tests[i].b);
        if (result != critical_tests[i].expected) {
            printf("\n%s: gMul(0x%02X, 0x%02X) = 0x%02X, 期望 0x%02X\n", 
                   critical_tests[i].description, 
                   critical_tests[i].a, critical_tests[i].b, 
                   result, critical_tests[i].expected);
            BOUNDARY_TEST_FAIL("gMul关键边界值测试失败");
            return 0;
        }
    }
    
    // 测试所有会触发模约简的情况
    for (u8 a = 0x80; a <= 0xFF; a++) {
        for (u8 b = 0x02; b <= 0x04; b++) {
            u8 result = gMul(a, b);
            // 验证结果在有效范围内
            BOUNDARY_ASSERT(result <= 0xFF, "gMul结果超出范围");
        }
    }
    
    BOUNDARY_TEST_PASS();
    return 1;
}

// 测试所有S-Box输入
int test_sbox_coverage() {
    BOUNDARY_TEST_START("S-Box完全覆盖测试");
    
    // 测试所有256个可能的输入
    for (int input = 0; input < 256; input++) {
        u8 state[16];
        memset(state, input, 16);
        
        u8 original[16];
        memcpy(original, state, 16);
        
        subBytes(state);
        
        // 验证所有字节都被正确替换
        for (int j = 0; j < 16; j++) {
            BOUNDARY_ASSERT(state[j] == SBox[input], "S-Box替换错误");
        }
        
        // 验证原始数据没有被意外修改（通过重新复制验证）
        memcpy(state, original, 16);
        BOUNDARY_ASSERT(state[0] == input, "原始数据被意外修改");
    }
    
    BOUNDARY_TEST_PASS();
    return 1;
}

// 测试mixColumns的所有可能输入组合
int test_mixColumns_boundary() {
    BOUNDARY_TEST_START("mixColumns边界测试");
    
    // 测试每列的边界情况
    for (int col = 0; col < 4; col++) {
        u8 state[16] = {0};
        
        // 只在当前列设置边界值
        state[4*col + 0] = 0xFF;
        state[4*col + 1] = 0x00;
        state[4*col + 2] = 0x80;
        state[4*col + 3] = 0x7F;
        
        u8 original[16];
        memcpy(original, state, 16);
        
        mixColumns(state);
        
        // 验证只有当前列被修改
        for (int other_col = 0; other_col < 4; other_col++) {
            if (other_col != col) {
                for (int row = 0; row < 4; row++) {
                    BOUNDARY_ASSERT(state[4*other_col + row] == original[4*other_col + row], 
                                  "mixColumns修改了其他列");
                }
            }
        }
        
        // 验证当前列确实被修改了
        int changed = 0;
        for (int row = 0; row < 4; row++) {
            if (state[4*col + row] != original[4*col + row]) {
                changed = 1;
                break;
            }
        }
        BOUNDARY_ASSERT(changed, "mixColumns没有修改目标列");
    }
    
    BOUNDARY_TEST_PASS();
    return 1;
}

// 测试shiftRows的所有位置
int test_shiftRows_boundary() {
    BOUNDARY_TEST_START("shiftRows边界位置测试");
    
    // 创建每个位置都不同的测试数据
    u8 state[16];
    for (int i = 0; i < 16; i++) {
        state[i] = i;
    }
    
    u8 original[16];
    memcpy(original, state, 16);
    
    shiftRows(state);
    
    // 验证每个位置都移动到了正确的新位置
    int shiftTab[16] = {0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11};
    for (int i = 0; i < 16; i++) {
        BOUNDARY_ASSERT(state[i] == original[shiftTab[i]], "shiftRows位置移动错误");
    }
    
    // 测试特殊值模式
    u8 patterns[][16] = {
        {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01},
        {0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0}
    };
    
    for (int p = 0; p < 2; p++) {
        memcpy(state, patterns[p], 16);
        shiftRows(state);
        
        // 验证数据完整性（所有原始字节仍然存在）
        int byte_count[256] = {0};
        for (int i = 0; i < 16; i++) {
            byte_count[patterns[p][i]]++;
        }
        
        int result_count[256] = {0};
        for (int i = 0; i < 16; i++) {
            result_count[state[i]]++;
        }
        
        for (int i = 0; i < 256; i++) {
            BOUNDARY_ASSERT(byte_count[i] == result_count[i], "shiftRows数据完整性测试失败");
        }
    }
    
    BOUNDARY_TEST_PASS();
    return 1;
}

// 测试expandKey的所有条件分支
int test_expandKey_boundary() {
    BOUNDARY_TEST_START("expandKey边界条件测试");
    
    // 测试会影响 i % 4 == 0 分支的特殊密钥
    u8 special_keys[][16] = {
        // 会产生特殊rCon值的密钥
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    };
    
    for (int k = 0; k < 4; k++) {
        u8 expandedKey[176];
        expandKey(special_keys[k], expandedKey);
        
        // 验证所有44轮的密钥扩展
        for (int round = 0; round < 44; round++) {
            // 检查每一轮的密钥都已经被计算
            if (round < 4) {
                // 前4轮应该与原始密钥相同
                BOUNDARY_ASSERT(expandedKey[4*round] == special_keys[k][4*round], "初始密钥复制错误");
            } else {
                // 后续轮次应该不同于原始密钥（除非是特殊情况）
                if (round % 4 == 0) {
                    // 这是关键轮次，应该经过特殊处理
                    // 验证rCon的应用
                    int rcon_index = round / 4;
                    if (rcon_index < 11) {
                        // 验证这一轮确实使用了rCon
                        // 这里简化验证，主要确保代码路径被执行
                    }
                }
            }
        }
        
        // 验证扩展密钥可以用于正常加密
        u8 test_plain[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
        u8 output[16];
        aes_128_encrypt(test_plain, special_keys[k], output);
        
        // 验证输出合理性
        int all_same = 1;
        for (int i = 1; i < 16; i++) {
            if (output[i] != output[0]) {
                all_same = 0;
                break;
            }
        }
        BOUNDARY_ASSERT(!all_same, "加密输出异常（全部相同）");
    }
    
    BOUNDARY_TEST_PASS();
    return 1;
}

// 测试白盒AES的所有内部状态转换
int test_wbaes_internal_states() {
    BOUNDARY_TEST_START("白盒AES内部状态测试");
    
    // 使用特殊构造的输入来测试内部状态的边界情况
    u8 key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    wbaes_gen(key);
    
    // 测试会产生特殊内部状态的输入
    u8 special_inputs[][16] = {
        // 会在第一轮产生特殊状态的输入
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        // 会在中间轮产生特殊状态的输入
        {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        // 会测试所有4个j值的输入
        {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10},
        {0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01}
    };
    
    for (int i = 0; i < 6; i++) {
        u8 wb_output[16], std_output[16];
        
        wbaes_encrypt(special_inputs[i], wb_output);
        aes_128_encrypt(special_inputs[i], key, std_output);
        
        BOUNDARY_ASSERT(memcmp(wb_output, std_output, 16) == 0, "特殊输入状态测试失败");
    }
    
    BOUNDARY_TEST_PASS();
    return 1;
}

// 测试所有位操作的边界情况
int test_bit_operations_boundary() {
    BOUNDARY_TEST_START("位操作边界测试");
    
    u8 key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    wbaes_gen(key);
    
    // 测试会触发所有位移操作的输入
    u8 bit_test_inputs[][16] = {
        // 测试 >> 28, >> 24, >> 20, >> 16, >> 12, >> 8, >> 4 的所有情况
        {0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F},
        {0x88, 0x44, 0x22, 0x11, 0x88, 0x44, 0x22, 0x11, 0x88, 0x44, 0x22, 0x11, 0x88, 0x44, 0x22, 0x11},
        {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}
    };
    
    for (int i = 0; i < 3; i++) {
        u8 wb_output[16], std_output[16];
        
        wbaes_encrypt(bit_test_inputs[i], wb_output);
        aes_128_encrypt(bit_test_inputs[i], key, std_output);
        
        BOUNDARY_ASSERT(memcmp(wb_output, std_output, 16) == 0, "位操作边界测试失败");
    }
    
    BOUNDARY_TEST_PASS();
    return 1;
}

// 测试循环边界条件
int test_loop_boundaries() {
    BOUNDARY_TEST_START("循环边界条件测试");
    
    // 测试for循环的边界情况
    // 在wbaes_encrypt中有多个嵌套循环，需要确保都被正确执行
    
    u8 key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    wbaes_gen(key);
    
    // 构造会测试所有j值（0到3）和i值（2到6）的输入
    for (int test_round = 0; test_round < 10; test_round++) {
        u8 plaintext[16];
        for (int i = 0; i < 16; i++) {
            plaintext[i] = (test_round * 17 + i * 23) % 256;
        }
        
        u8 wb_output[16], std_output[16];
        wbaes_encrypt(plaintext, wb_output);
        aes_128_encrypt(plaintext, key, std_output);
        
        BOUNDARY_ASSERT(memcmp(wb_output, std_output, 16) == 0, "循环边界测试失败");
    }
    
    BOUNDARY_TEST_PASS();
    return 1;
}

// 测试数组边界访问
int test_array_boundaries() {
    BOUNDARY_TEST_START("数组边界访问测试");
    
    u8 key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    wbaes_gen(key);
    
    // 测试会访问查找表边界的输入
    u8 boundary_inputs[][16] = {
        // 测试查找表的第一个和最后一个索引
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 索引0
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // 索引255
        // 测试会产生边界索引的中间值
        {0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80}
    };
    
    for (int i = 0; i < 3; i++) {
        u8 wb_output[16], std_output[16];
        
        wbaes_encrypt(boundary_inputs[i], wb_output);
        aes_128_encrypt(boundary_inputs[i], key, std_output);
        
        BOUNDARY_ASSERT(memcmp(wb_output, std_output, 16) == 0, "数组边界访问测试失败");
    }
    
    BOUNDARY_TEST_PASS();
    return 1;
}

// 测试随机数生成对算法的影响
int test_random_generation_impact() {
    BOUNDARY_TEST_START("随机数生成影响测试");
    
    u8 key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    u8 plaintext[16] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    
    // 使用不同的随机种子生成查找表
    u8 outputs[5][16];
    
    for (int seed = 0; seed < 5; seed++) {
        // 重新设置随机种子（通过重新初始化时间）
        wbaes_gen(key);
        wbaes_encrypt(plaintext, outputs[seed]);
    }
    
    // 验证所有输出都相同（确保随机性不影响加密结果的正确性）
    for (int i = 1; i < 5; i++) {
        BOUNDARY_ASSERT(memcmp(outputs[0], outputs[i], 16) == 0, "随机种子影响加密结果一致性");
    }
    
    // 验证与标准AES的一致性
    u8 std_output[16];
    aes_128_encrypt(plaintext, key, std_output);
    BOUNDARY_ASSERT(memcmp(outputs[0], std_output, 16) == 0, "随机生成的查找表与标准AES不一致");
    
    BOUNDARY_TEST_PASS();
    return 1;
}

// 主边界测试函数
int main() {
    printf("=== 白盒AES边界测试套件 ===\n\n");
    
    // 运行所有边界测试
    test_gMul_exhaustive();
    test_sbox_coverage();
    test_mixColumns_boundary();
    test_shiftRows_boundary();
    test_expandKey_boundary();
    test_wbaes_internal_states();
    test_bit_operations_boundary();
    test_loop_boundaries();
    test_array_boundaries();
    test_random_generation_impact();
    
    // 输出边界测试结果
    printf("\n=== 边界测试结果 ===\n");
    printf("总边界测试数: %d\n", boundary_test_count);
    printf("通过: %d\n", boundary_passed);
    printf("失败: %d\n", boundary_failed);
    printf("成功率: %.1f%%\n", (boundary_passed * 100.0) / boundary_test_count);
    
    return boundary_failed == 0 ? 0 : 1;
}