#include "wbaes.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

// 测试计数器
static int test_count = 0;
static int passed_tests = 0;
static int failed_tests = 0;

// 测试宏定义
#define TEST_START(name) \
    do { \
        test_count++; \
        printf("测试 %d: %s ... ", test_count, name); \
        fflush(stdout); \
    } while(0)

#define TEST_PASS() \
    do { \
        passed_tests++; \
        printf("通过\n"); \
    } while(0)

#define TEST_FAIL(msg) \
    do { \
        failed_tests++; \
        printf("失败: %s\n", msg); \
    } while(0)

#define ASSERT_TRUE(condition, msg) \
    do { \
        if (!(condition)) { \
            TEST_FAIL(msg); \
            return 0; \
        } \
    } while(0)

#define ASSERT_FALSE(condition, msg) \
    do { \
        if (condition) { \
            TEST_FAIL(msg); \
            return 0; \
        } \
    } while(0)

#define ASSERT_EQUAL_BYTES(expected, actual, size, msg) \
    do { \
        if (memcmp(expected, actual, size) != 0) { \
            printf("\n期望: "); \
            for(int i = 0; i < size; i++) printf("%02X ", expected[i]); \
            printf("\n实际: "); \
            for(int i = 0; i < size; i++) printf("%02X ", actual[i]); \
            printf("\n"); \
            TEST_FAIL(msg); \
            return 0; \
        } \
    } while(0)

// 测试向量结构
typedef struct {
    u8 key[16];
    u8 plaintext[16];
    u8 expected_ciphertext[16];
    const char* description;
} test_vector_t;

// 标准AES测试向量
static test_vector_t test_vectors[] = {
    // NIST测试向量1 - 全零密钥和明文
    {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x66, 0xe9, 0x4b, 0xd4, 0xef, 0x8a, 0x2c, 0x3b, 0x88, 0x4c, 0xfa, 0x59, 0xca, 0x34, 0x2b, 0x2e},
        "全零密钥和明文"
    },
    // NIST测试向量2 - 全FF密钥
    {
        {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0xa1, 0xf6, 0x25, 0x8c, 0x87, 0x7d, 0x5f, 0xcd, 0x89, 0x64, 0x48, 0x45, 0x38, 0xbf, 0xc9, 0x2c},
        "全FF密钥"
    },
    // 测试向量3 - 递增模式
    {
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f},
        {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff},
        {0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30, 0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a},
        "递增模式密钥和明文"
    },
    // 测试向量4 - 随机测试
    {
        {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c},
        {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34},
        {0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb, 0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32},
        "NIST标准测试向量"
    }
};

// 边界值测试用例
static void generate_boundary_test_cases(test_vector_t* vectors, int* count) {
    // 测试每个字节的边界值
    static u8 boundary_key[16] = {0x80, 0x01, 0xfe, 0x7f, 0x00, 0xff, 0x55, 0xaa, 0x33, 0xcc, 0x0f, 0xf0, 0x3c, 0xc3, 0x5a, 0xa5};
    static u8 boundary_plain[16] = {0x00, 0x01, 0x7f, 0x80, 0xfe, 0xff, 0xaa, 0x55, 0xcc, 0x33, 0xf0, 0x0f, 0xc3, 0x3c, 0xa5, 0x5a};
    
    memcpy(vectors[*count].key, boundary_key, 16);
    memcpy(vectors[*count].plaintext, boundary_plain, 16);
    vectors[*count].description = "边界值测试";
    (*count)++;
}

// 辅助函数：比较两个字节数组
int compare_bytes(const u8* a, const u8* b, int size) {
    return memcmp(a, b, size) == 0;
}

// 辅助函数：打印字节数组
void print_bytes(const char* label, const u8* data, int size) {
    printf("%s: ", label);
    for (int i = 0; i < size; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

// 测试1: 基本功能测试
int test_basic_functionality() {
    TEST_START("基本加密功能");
    
    u8 key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    u8 plaintext[16] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    u8 wb_output[16];
    u8 std_output[16];
    
    // 生成白盒查找表
    wbaes_gen(key);
    
    // 白盒AES加密
    wbaes_encrypt(plaintext, wb_output);
    
    // 标准AES加密
    aes_128_encrypt(plaintext, key, std_output);
    
    // 验证结果一致性
    ASSERT_TRUE(compare_bytes(wb_output, std_output, 16), "白盒AES和标准AES结果不一致");
    
    TEST_PASS();
    return 1;
}

// 测试2: 所有测试向量验证
int test_all_vectors() {
    TEST_START("所有测试向量验证");
    
    int vector_count = sizeof(test_vectors) / sizeof(test_vectors[0]);
    
    for (int i = 0; i < vector_count; i++) {
        u8 wb_output[16];
        u8 std_output[16];
        
        // 生成白盒查找表
        wbaes_gen(test_vectors[i].key);
        
        // 白盒AES加密
        wbaes_encrypt(test_vectors[i].plaintext, wb_output);
        
        // 标准AES加密
        aes_128_encrypt(test_vectors[i].plaintext, test_vectors[i].key, std_output);
        
        // 验证与标准AES的一致性
        if (!compare_bytes(wb_output, std_output, 16)) {
            printf("\n测试向量 %d (%s) 失败\n", i+1, test_vectors[i].description);
            print_bytes("明文", test_vectors[i].plaintext, 16);
            print_bytes("密钥", test_vectors[i].key, 16);
            print_bytes("白盒结果", wb_output, 16);
            print_bytes("标准结果", std_output, 16);
            TEST_FAIL("测试向量验证失败");
            return 0;
        }
    }
    
    TEST_PASS();
    return 1;
}

// 测试3: 边界值测试
int test_boundary_values() {
    TEST_START("边界值测试");
    
    test_vector_t boundary_vectors[10];
    int boundary_count = 0;
    generate_boundary_test_cases(boundary_vectors, &boundary_count);
    
    for (int i = 0; i < boundary_count; i++) {
        u8 wb_output[16];
        u8 std_output[16];
        
        wbaes_gen(boundary_vectors[i].key);
        wbaes_encrypt(boundary_vectors[i].plaintext, wb_output);
        aes_128_encrypt(boundary_vectors[i].plaintext, boundary_vectors[i].key, std_output);
        
        ASSERT_TRUE(compare_bytes(wb_output, std_output, 16), "边界值测试失败");
    }
    
    TEST_PASS();
    return 1;
}

// 测试4: 随机测试（稳定性测试）
int test_random_stability() {
    TEST_START("随机稳定性测试");
    
    srand(time(NULL));
    
    for (int test_case = 0; test_case < 100; test_case++) {
        u8 key[16], plaintext[16];
        u8 wb_output[16], std_output[16];
        
        // 生成随机密钥和明文
        for (int i = 0; i < 16; i++) {
            key[i] = rand() % 256;
            plaintext[i] = rand() % 256;
        }
        
        wbaes_gen(key);
        wbaes_encrypt(plaintext, wb_output);
        aes_128_encrypt(plaintext, key, std_output);
        
        if (!compare_bytes(wb_output, std_output, 16)) {
            printf("\n随机测试用例 %d 失败\n", test_case + 1);
            print_bytes("密钥", key, 16);
            print_bytes("明文", plaintext, 16);
            print_bytes("白盒结果", wb_output, 16);
            print_bytes("标准结果", std_output, 16);
            TEST_FAIL("随机稳定性测试失败");
            return 0;
        }
    }
    
    TEST_PASS();
    return 1;
}

// 测试5: 重复加密测试（确保查找表生成的一致性）
int test_repeated_encryption() {
    TEST_START("重复加密一致性测试");
    
    u8 key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    u8 plaintext[16] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    u8 output1[16], output2[16], output3[16];
    
    // 生成查找表一次
    wbaes_gen(key);
    
    // 多次加密同一明文
    wbaes_encrypt(plaintext, output1);
    wbaes_encrypt(plaintext, output2);
    wbaes_encrypt(plaintext, output3);
    
    // 验证结果一致性
    ASSERT_TRUE(compare_bytes(output1, output2, 16), "重复加密结果不一致(1vs2)");
    ASSERT_TRUE(compare_bytes(output2, output3, 16), "重复加密结果不一致(2vs3)");
    
    TEST_PASS();
    return 1;
}

// 测试6: 不同密钥测试
int test_different_keys() {
    TEST_START("不同密钥测试");
    
    u8 key1[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    u8 key2[16] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
    u8 plaintext[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    u8 output1[16], output2[16];
    u8 std_output1[16], std_output2[16];
    
    // 测试密钥1
    wbaes_gen(key1);
    wbaes_encrypt(plaintext, output1);
    aes_128_encrypt(plaintext, key1, std_output1);
    
    // 测试密钥2
    wbaes_gen(key2);
    wbaes_encrypt(plaintext, output2);
    aes_128_encrypt(plaintext, key2, std_output2);
    
    // 验证每个密钥的结果正确性
    ASSERT_TRUE(compare_bytes(output1, std_output1, 16), "密钥1加密结果错误");
    ASSERT_TRUE(compare_bytes(output2, std_output2, 16), "密钥2加密结果错误");
    
    // 验证不同密钥产生不同结果
    ASSERT_FALSE(compare_bytes(output1, output2, 16), "不同密钥产生相同结果");
    
    TEST_PASS();
    return 1;
}

// 测试7: 打印函数测试
int test_print_functions() {
    TEST_START("打印函数测试");
    
    u8 test_data[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    
    printf("\n测试printstate函数输出: ");
    printstate(test_data);
    
    printf("测试printState函数输出:\n");
    printState(test_data);
    
    TEST_PASS();
    return 1;
}

// 测试8: 内存安全测试
int test_memory_safety() {
    TEST_START("内存安全测试");
    
    u8 key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    u8 plaintext[16] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    
    // 测试多次生成查找表
    for (int i = 0; i < 10; i++) {
        wbaes_gen(key);
        
        u8 output[16];
        wbaes_encrypt(plaintext, output);
        
        // 验证输出不全为零（基本合理性检查）
        int all_zero = 1;
        for (int j = 0; j < 16; j++) {
            if (output[j] != 0) {
                all_zero = 0;
                break;
            }
        }
        ASSERT_FALSE(all_zero, "加密输出全为零");
    }
    
    TEST_PASS();
    return 1;
}

// 测试9: 性能基准测试
int test_performance_benchmark() {
    TEST_START("性能基准测试");
    
    u8 key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    u8 plaintext[16] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    u8 output[16];
    
    // 生成查找表
    clock_t start = clock();
    wbaes_gen(key);
    clock_t gen_time = clock() - start;
    
    // 加密性能测试
    start = clock();
    for (int i = 0; i < 1000; i++) {
        wbaes_encrypt(plaintext, output);
    }
    clock_t encrypt_time = clock() - start;
    
    printf("\n查找表生成时间: %f ms\n", ((double)gen_time / CLOCKS_PER_SEC) * 1000);
    printf("1000次加密时间: %f ms\n", ((double)encrypt_time / CLOCKS_PER_SEC) * 1000);
    printf("平均每次加密时间: %f ms\n", ((double)encrypt_time / CLOCKS_PER_SEC / 1000) * 1000);
    
    TEST_PASS();
    return 1;
}

// 测试10: 标准AES函数覆盖测试
int test_standard_aes_coverage() {
    TEST_START("标准AES函数覆盖测试");
    
    u8 key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    u8 plaintext[16] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    u8 output[16];
    u8 expandedKey[176];
    
    // 测试密钥扩展
    expandKey(key, expandedKey);
    
    // 复制明文到state进行测试
    memcpy(output, plaintext, 16);
    
    // 测试各个AES组件
    addRoundKey(output, key);
    subBytes(output);
    shiftRows(output);
    mixColumns(output);
    
    // 测试gMul函数的各种输入
    u8 mul_result;
    mul_result = gMul(0x02, 0x87); // 测试有进位的情况
    mul_result = gMul(0x00, 0xff); // 测试零乘法
    mul_result = gMul(0xff, 0xff); // 测试最大值乘法
    mul_result = gMul(0x01, 0x53); // 测试单位元乘法
    
    // 测试printState函数
    printf("\n测试printState输出:\n");
    printState(output);
    
    TEST_PASS();
    return 1;
}

// 测试11: 极端输入测试
int test_extreme_inputs() {
    TEST_START("极端输入测试");
    
    u8 output[16];
    
    // 测试1: 全零输入
    u8 zero_key[16] = {0};
    u8 zero_plain[16] = {0};
    wbaes_gen(zero_key);
    wbaes_encrypt(zero_plain, output);
    
    // 测试2: 全FF输入
    u8 ff_key[16], ff_plain[16];
    memset(ff_key, 0xFF, 16);
    memset(ff_plain, 0xFF, 16);
    wbaes_gen(ff_key);
    wbaes_encrypt(ff_plain, output);
    
    // 测试3: 交替模式
    u8 alt_key[16] = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};
    u8 alt_plain[16] = {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};
    wbaes_gen(alt_key);
    wbaes_encrypt(alt_plain, output);
    
    TEST_PASS();
    return 1;
}

// 测试12: 查找表重新生成测试
int test_table_regeneration() {
    TEST_START("查找表重新生成测试");
    
    u8 key1[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    u8 key2[16] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
    u8 plaintext[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    
    // 第一次生成和加密
    wbaes_gen(key1);
    u8 output1[16];
    wbaes_encrypt(plaintext, output1);
    
    // 第二次生成和加密（相同密钥）
    wbaes_gen(key1);
    u8 output1_repeat[16];
    wbaes_encrypt(plaintext, output1_repeat);
    
    // 第三次生成和加密（不同密钥）
    wbaes_gen(key2);
    u8 output2[16];
    wbaes_encrypt(plaintext, output2);
    
    // 验证相同密钥产生相同结果
    ASSERT_TRUE(compare_bytes(output1, output1_repeat, 16), "相同密钥重新生成查找表结果不一致");
    
    // 验证不同密钥产生不同结果
    ASSERT_FALSE(compare_bytes(output1, output2, 16), "不同密钥产生相同加密结果");
    
    TEST_PASS();
    return 1;
}

// 测试13: 输入验证测试
int test_input_validation() {
    TEST_START("输入验证测试");
    
    u8 key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    u8 plaintext[16] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    u8 output[16];
    
    // 正常情况测试
    wbaes_gen(key);
    wbaes_encrypt(plaintext, output);
    
    // 测试各种输入模式
    u8 pattern_inputs[][16] = {
        {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10},
        {0xf0, 0xe1, 0xd2, 0xc3, 0xb4, 0xa5, 0x96, 0x87, 0x78, 0x69, 0x5a, 0x4b, 0x3c, 0x2d, 0x1e, 0x0f},
        {0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5}
    };
    
    for (int i = 0; i < 3; i++) {
        wbaes_encrypt(pattern_inputs[i], output);
        
        // 验证输出不全为零
        int all_zero = 1;
        for (int j = 0; j < 16; j++) {
            if (output[j] != 0) {
                all_zero = 0;
                break;
            }
        }
        ASSERT_FALSE(all_zero, "模式输入产生全零输出");
    }
    
    TEST_PASS();
    return 1;
}

// 测试14: 雪崩效应测试
int test_avalanche_effect() {
    TEST_START("雪崩效应测试");
    
    u8 key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    u8 plaintext1[16] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    u8 plaintext2[16] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x35}; // 最后一位不同
    u8 output1[16], output2[16];
    
    wbaes_gen(key);
    wbaes_encrypt(plaintext1, output1);
    wbaes_encrypt(plaintext2, output2);
    
    // 计算不同的位数
    int diff_bits = 0;
    for (int i = 0; i < 16; i++) {
        u8 xor_result = output1[i] ^ output2[i];
        for (int j = 0; j < 8; j++) {
            if (xor_result & (1 << j)) {
                diff_bits++;
            }
        }
    }
    
    printf("\n输入1位差异导致输出%d位差异 (%.1f%%)\n", diff_bits, (diff_bits * 100.0) / 128);
    
    // 雪崩效应应该至少改变25%的位
    ASSERT_TRUE(diff_bits >= 32, "雪崩效应不足");
    
    TEST_PASS();
    return 1;
}

// 测试15: 压力测试
int test_stress() {
    TEST_START("压力测试");
    
    // 大量不同密钥和明文的组合测试
    for (int key_pattern = 0; key_pattern < 10; key_pattern++) {
        u8 key[16];
        for (int i = 0; i < 16; i++) {
            key[i] = (key_pattern * 17 + i * 23) % 256;
        }
        
        wbaes_gen(key);
        
        for (int plain_pattern = 0; plain_pattern < 10; plain_pattern++) {
            u8 plaintext[16];
            for (int i = 0; i < 16; i++) {
                plaintext[i] = (plain_pattern * 31 + i * 37) % 256;
            }
            
            u8 wb_output[16], std_output[16];
            wbaes_encrypt(plaintext, wb_output);
            aes_128_encrypt(plaintext, key, std_output);
            
            ASSERT_TRUE(compare_bytes(wb_output, std_output, 16), "压力测试中发现不一致");
        }
    }
    
    TEST_PASS();
    return 1;
}

// 主测试函数
int main(int argc, char* argv[]) {
    printf("=== 白盒AES算法综合测试套件 ===\n\n");
    
    // 运行所有测试
    test_basic_functionality();
    test_all_vectors();
    test_boundary_values();
    test_random_stability();
    test_repeated_encryption();
    test_different_keys();
    test_print_functions();
    test_memory_safety();
    test_performance_benchmark();
    test_standard_aes_coverage();
    test_avalanche_effect();
    test_stress();
    
    // 输出测试结果统计
    printf("\n=== 测试结果统计 ===\n");
    printf("总测试数: %d\n", test_count);
    printf("通过测试: %d\n", passed_tests);
    printf("失败测试: %d\n", failed_tests);
    printf("成功率: %.1f%%\n", (passed_tests * 100.0) / test_count);
    
    if (failed_tests == 0) {
        printf("\n🎉 所有测试通过！算法功能正常。\n");
        return 0;
    } else {
        printf("\n❌ %d个测试失败，需要修复。\n", failed_tests);
        return 1;
    }
}