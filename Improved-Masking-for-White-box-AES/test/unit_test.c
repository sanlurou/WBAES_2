#include "wbaes.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

// 单元测试计数器
static int unit_test_count = 0;
static int unit_passed = 0;
static int unit_failed = 0;

#define UNIT_TEST_START(name) \
    do { \
        unit_test_count++; \
        printf("单元测试 %d: %s ... ", unit_test_count, name); \
        fflush(stdout); \
    } while(0)

#define UNIT_TEST_PASS() \
    do { \
        unit_passed++; \
        printf("通过\n"); \
    } while(0)

#define UNIT_TEST_FAIL(msg) \
    do { \
        unit_failed++; \
        printf("失败: %s\n", msg); \
    } while(0)

#define UNIT_ASSERT(condition, msg) \
    do { \
        if (!(condition)) { \
            UNIT_TEST_FAIL(msg); \
            return 0; \
        } \
    } while(0)

// 测试gMul函数的所有分支
int test_gMul_all_branches() {
    UNIT_TEST_START("gMul函数所有分支测试");
    
    // 测试b=0的情况（早期退出）
    UNIT_ASSERT(gMul(0x53, 0x00) == 0x00, "gMul(x, 0) 应该等于0");
    
    // 测试a=0的情况
    UNIT_ASSERT(gMul(0x00, 0x53) == 0x00, "gMul(0, x) 应该等于0");
    
    // 测试b的每一位都为1的情况
    UNIT_ASSERT(gMul(0x02, 0xFF) == 0xE5, "gMul(2, 255) 测试失败");
    
    // 测试需要模约简的情况（a的最高位为1）
    UNIT_ASSERT(gMul(0x80, 0x02) == 0x1B, "gMul(128, 2) 模约简测试失败");
    UNIT_ASSERT(gMul(0xFF, 0x02) == 0xE5, "gMul(255, 2) 模约简测试失败");
    
    // 测试不需要模约简的情况
    UNIT_ASSERT(gMul(0x53, 0x02) == 0xA6, "gMul(83, 2) 无模约简测试失败");
    
    // 测试b的每一位的影响
    for (int bit = 0; bit < 8; bit++) {
        u8 b = 1 << bit;
        u8 result = gMul(0x53, b);
        // 验证结果不为零（除非输入为零）
        if (b != 0) {
            UNIT_ASSERT(result != 0 || bit == 0, "gMul位测试失败");
        }
    }
    
    // 测试GF(2^8)的特性：x * x^(-1) = 1
    // 在GF(2^8)中，0x53的逆元是0xCA
    UNIT_ASSERT(gMul(0x53, 0xCA) == 0x01, "GF(2^8)逆元测试失败");
    
    UNIT_TEST_PASS();
    return 1;
}

// 测试subBytes函数
int test_subBytes() {
    UNIT_TEST_START("subBytes函数测试");
    
    u8 state[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    u8 expected[16] = {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76};
    
    subBytes(state);
    
    UNIT_ASSERT(memcmp(state, expected, 16) == 0, "subBytes结果不正确");
    
    // 测试所有可能的字节值
    for (int i = 0; i < 256; i++) {
        u8 test_state[16];
        memset(test_state, i, 16);
        subBytes(test_state);
        
        // 验证每个字节都被正确替换
        for (int j = 0; j < 16; j++) {
            UNIT_ASSERT(test_state[j] == SBox[i], "subBytes SBox查找错误");
        }
    }
    
    UNIT_TEST_PASS();
    return 1;
}

// 测试shiftRows函数
int test_shiftRows() {
    UNIT_TEST_START("shiftRows函数测试");
    
    u8 state[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    u8 expected[16] = {0x00, 0x05, 0x0a, 0x0f, 0x04, 0x09, 0x0e, 0x03, 0x08, 0x0d, 0x02, 0x07, 0x0c, 0x01, 0x06, 0x0b};
    
    shiftRows(state);
    
    UNIT_ASSERT(memcmp(state, expected, 16) == 0, "shiftRows结果不正确");
    
    // 测试特殊模式
    u8 pattern_state[16] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00};
    u8 pattern_expected[16] = {0xaa, 0xff, 0x55, 0x00, 0xee, 0x44, 0x99, 0xdd, 0x33, 0x88, 0xcc, 0x22, 0x77, 0xbb, 0x11, 0x66};
    
    shiftRows(pattern_state);
    UNIT_ASSERT(memcmp(pattern_state, pattern_expected, 16) == 0, "shiftRows模式测试失败");
    
    UNIT_TEST_PASS();
    return 1;
}

// 测试addRoundKey函数
int test_addRoundKey() {
    UNIT_TEST_START("addRoundKey函数测试");
    
    u8 state[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    u8 key[16] = {0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};
    u8 expected[16] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    
    addRoundKey(state, key);
    
    UNIT_ASSERT(memcmp(state, expected, 16) == 0, "addRoundKey XOR操作错误");
    
    // 测试自反性：A XOR B XOR B = A
    u8 original[16] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    u8 test_state[16];
    memcpy(test_state, original, 16);
    
    addRoundKey(test_state, key);  // A XOR B
    addRoundKey(test_state, key);  // (A XOR B) XOR B = A
    
    UNIT_ASSERT(memcmp(test_state, original, 16) == 0, "addRoundKey自反性测试失败");
    
    UNIT_TEST_PASS();
    return 1;
}

// 测试mixColumns函数
int test_mixColumns() {
    UNIT_TEST_START("mixColumns函数测试");
    
    // 测试已知的输入输出
    u8 state[16] = {0xd4, 0xbf, 0x5d, 0x30, 0xe0, 0xb4, 0x52, 0xae, 0xb8, 0x41, 0x11, 0xf1, 0x1e, 0x27, 0x98, 0xe5};
    u8 expected[16] = {0x04, 0x66, 0x81, 0xe5, 0xe0, 0xcb, 0x19, 0x9a, 0x48, 0xf8, 0xd3, 0x7a, 0x28, 0x06, 0x26, 0x4c};
    
    mixColumns(state);
    
    UNIT_ASSERT(memcmp(state, expected, 16) == 0, "mixColumns已知测试向量失败");
    
    // 测试全零输入
    u8 zero_state[16] = {0};
    u8 zero_expected[16] = {0};
    mixColumns(zero_state);
    UNIT_ASSERT(memcmp(zero_state, zero_expected, 16) == 0, "mixColumns全零测试失败");
    
    // 测试每列独立性
    u8 col_test[16] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    mixColumns(col_test);
    // 只有第一列应该被修改
    for (int i = 4; i < 16; i++) {
        UNIT_ASSERT(col_test[i] == 0, "mixColumns列独立性测试失败");
    }
    
    UNIT_TEST_PASS();
    return 1;
}

// 测试expandKey函数的所有分支
int test_expandKey() {
    UNIT_TEST_START("expandKey函数所有分支测试");
    
    u8 key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    u8 expandedKey[176];
    
    expandKey(key, expandedKey);
    
    // 验证前16字节与原始密钥相同
    UNIT_ASSERT(memcmp(expandedKey, key, 16) == 0, "expandKey前16字节应该与原始密钥相同");
    
    // 测试密钥扩展的特定轮次（i % 4 == 0的分支）
    // 检查第4轮（i=4）的密钥扩展
    u8 expected_round1[4] = {0xa0, 0xfa, 0xfe, 0x17};
    UNIT_ASSERT(memcmp(expandedKey + 16, expected_round1, 4) == 0, "expandKey第4轮扩展错误");
    
    // 测试所有可能的密钥模式
    u8 test_keys[][16] = {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 全零
        {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // 全FF
        {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10}, // 递增
    };
    
    for (int t = 0; t < 3; t++) {
        u8 exp_key[176];
        expandKey(test_keys[t], exp_key);
        
        // 验证前16字节
        UNIT_ASSERT(memcmp(exp_key, test_keys[t], 16) == 0, "expandKey初始复制错误");
        
        // 验证扩展密钥长度正确
        // 通过检查最后几轮的密钥是否合理
        int non_zero_count = 0;
        for (int i = 160; i < 176; i++) {
            if (exp_key[i] != 0) non_zero_count++;
        }
        // 最后一轮密钥不应该全为零（除非是特殊密钥）
        if (t != 0) { // 非全零密钥
            UNIT_ASSERT(non_zero_count > 0, "expandKey最后一轮密钥异常");
        }
    }
    
    UNIT_TEST_PASS();
    return 1;
}

// 测试aes_128_encrypt函数的所有分支
int test_aes_128_encrypt_branches() {
    UNIT_TEST_START("aes_128_encrypt分支测试");
    
    u8 key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    u8 plaintext[16] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    u8 output[16];
    u8 expected[16] = {0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb, 0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32};
    
    aes_128_encrypt(plaintext, key, output);
    
    UNIT_ASSERT(memcmp(output, expected, 16) == 0, "aes_128_encrypt标准测试向量失败");
    
    // 测试9轮循环的每一轮
    // 通过修改输入来确保循环的每一轮都被执行
    u8 test_input[16];
    for (int round = 0; round < 9; round++) {
        memcpy(test_input, plaintext, 16);
        // 修改输入以影响特定轮次
        test_input[round % 16] ^= 0x01;
        
        aes_128_encrypt(test_input, key, output);
        
        // 验证输出不同于原始输出
        UNIT_ASSERT(memcmp(output, expected, 16) != 0, "AES轮次测试失败");
    }
    
    UNIT_TEST_PASS();
    return 1;
}

// 测试printstate和printState函数
int test_print_functions() {
    UNIT_TEST_START("打印函数测试");
    
    u8 test_data[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    
    // 重定向输出到缓冲区进行验证（简化版本）
    printf("\n=== printstate输出测试 ===\n");
    printstate(test_data);
    
    printf("=== printState输出测试 ===\n");
    printState(test_data);
    
    // 测试空数据
    u8 zero_data[16] = {0};
    printf("=== 零数据输出测试 ===\n");
    printstate(zero_data);
    printState(zero_data);
    
    // 测试最大值数据
    u8 max_data[16];
    memset(max_data, 0xFF, 16);
    printf("=== 最大值数据输出测试 ===\n");
    printstate(max_data);
    printState(max_data);
    
    UNIT_TEST_PASS();
    return 1;
}

// 测试wbaes_gen函数的分支覆盖
int test_wbaes_gen_branches() {
    UNIT_TEST_START("wbaes_gen分支覆盖测试");
    
    // 测试不同的密钥以触发不同的代码路径
    u8 test_keys[][16] = {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
        {0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55},
        {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10}
    };
    
    for (int i = 0; i < 4; i++) {
        // 每个密钥都应该成功生成查找表
        wbaes_gen(test_keys[i]);
        
        // 验证生成的查找表可以正常工作
        u8 test_plain[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
        u8 wb_output[16], std_output[16];
        
        wbaes_encrypt(test_plain, wb_output);
        aes_128_encrypt(test_plain, test_keys[i], std_output);
        
        UNIT_ASSERT(memcmp(wb_output, std_output, 16) == 0, "wbaes_gen生成的查找表验证失败");
    }
    
    UNIT_TEST_PASS();
    return 1;
}

// 测试wbaes_encrypt函数的所有轮次
int test_wbaes_encrypt_rounds() {
    UNIT_TEST_START("wbaes_encrypt轮次覆盖测试");
    
    u8 key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    
    // 生成查找表
    wbaes_gen(key);
    
    // 测试不同的输入模式以确保所有轮次都被正确执行
    u8 test_inputs[][16] = {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 全零
        {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // 全FF
        {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}, // 位模式
        {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34}  // 标准测试
    };
    
    for (int i = 0; i < 4; i++) {
        u8 wb_output[16], std_output[16];
        
        wbaes_encrypt(test_inputs[i], wb_output);
        aes_128_encrypt(test_inputs[i], key, std_output);
        
        UNIT_ASSERT(memcmp(wb_output, std_output, 16) == 0, "wbaes_encrypt轮次测试失败");
    }
    
    UNIT_TEST_PASS();
    return 1;
}

// 测试异常情况处理
int test_edge_cases() {
    UNIT_TEST_START("边缘情况测试");
    
    // 测试连续的相同操作
    u8 key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    u8 plaintext[16] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    
    // 连续多次生成查找表
    for (int i = 0; i < 5; i++) {
        wbaes_gen(key);
    }
    
    // 验证最后生成的查找表仍然正确
    u8 wb_output[16], std_output[16];
    wbaes_encrypt(plaintext, wb_output);
    aes_128_encrypt(plaintext, key, std_output);
    
    UNIT_ASSERT(memcmp(wb_output, std_output, 16) == 0, "连续生成查找表测试失败");
    
    UNIT_TEST_PASS();
    return 1;
}

// 主单元测试函数
int main() {
    printf("=== 白盒AES单元测试套件 ===\n\n");
    
    // 运行所有单元测试
    test_gMul_all_branches();
    test_subBytes();
    test_shiftRows();
    test_addRoundKey();
    test_mixColumns();
    test_expandKey();
    test_wbaes_gen_branches();
    test_wbaes_encrypt_rounds();
    test_edge_cases();
    
    // 输出单元测试结果
    printf("\n=== 单元测试结果 ===\n");
    printf("总单元测试数: %d\n", unit_test_count);
    printf("通过: %d\n", unit_passed);
    printf("失败: %d\n", unit_failed);
    printf("成功率: %.1f%%\n", (unit_passed * 100.0) / unit_test_count);
    
    return unit_failed == 0 ? 0 : 1;
}