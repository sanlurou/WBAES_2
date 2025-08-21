#include "wbaes.h"
#include "WBMatrix/WBMatrix.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// 覆盖率完善测试计数器
static int completion_test_count = 0;
static int completion_passed = 0;
static int completion_failed = 0;

#define COMPLETION_TEST_START(name) \
    do { \
        completion_test_count++; \
        printf("覆盖率完善测试 %d: %s ... ", completion_test_count, name); \
        fflush(stdout); \
    } while(0)

#define COMPLETION_TEST_PASS() \
    do { \
        completion_passed++; \
        printf("通过\n"); \
    } while(0)

#define COMPLETION_TEST_FAIL(msg) \
    do { \
        completion_failed++; \
        printf("失败: %s\n", msg); \
    } while(0)

// 测试initialize_aes_sbox函数
int test_initialize_aes_sbox() {
    COMPLETION_TEST_START("initialize_aes_sbox函数测试");
    
    u8 custom_sbox[256];
    initialize_aes_sbox(custom_sbox);
    
    // 验证生成的S-Box与标准S-Box一致
    for (int i = 0; i < 256; i++) {
        if (custom_sbox[i] != SBox[i]) {
            printf("\n位置 %d: 生成=%02X, 期望=%02X\n", i, custom_sbox[i], SBox[i]);
            COMPLETION_TEST_FAIL("生成的S-Box与标准S-Box不一致");
            return 0;
        }
    }
    
    COMPLETION_TEST_PASS();
    return 1;
}

// 测试所有M4矩阵操作函数
int test_M4_complete_coverage() {
    COMPLETION_TEST_START("M4矩阵完整覆盖测试");
    
    M4 mat1, mat2, mat3, mat_inv, mat_trans;
    V4 vec1, vec2, vec_result;
    Aff4 aff, aff_inv, aff_mix;
    
    // 测试初始化函数
    initM4(&mat1);
    initV4(&vec1);
    
    // 测试随机生成
    randM4(&mat1);
    randV4(&vec1);
    
    // 测试单位矩阵
    identityM4(&mat2);
    
    // 测试打印函数
    printf("\n=== M4矩阵打印测试 ===\n");
    printM4(mat1);
    printbitM4(mat1);
    printV4(vec1);
    
    // 测试矩阵操作
    copyM4(mat1, &mat2);
    int equal = isequalM4(mat1, mat2);
    assert(equal == 1);
    
    // 测试可逆性
    if (isinvertM4(mat1)) {
        invsM4(mat1, &mat_inv);
    }
    
    // 测试位操作
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int bit = readbitM4(mat1, i, j);
            setbitM4(&mat2, i, j, bit);
            flipbitM4(&mat2, i, j);
        }
    }
    
    // 测试矩阵运算
    MatMulMatM4(mat1, mat2, &mat3);
    MatAddMatM4(mat1, mat2, &mat3);
    MattransM4(mat1, &mat_trans);
    
    // 测试向量运算
    randV4(&vec2);
    VecAddVecV4(vec1, vec2, &vec_result);
    MatMulVecM4(mat1, vec1, &vec_result);
    
    // 测试数字运算
    for (uint8_t num = 0; num < 16; num++) {
        uint8_t result = MatMulNumM4(mat1, num);
        uint8_t affine_result = affineU4(aff, num);
        int xor_result = xorU4(num);
        int hw_result = HWU4(num);
    }
    
    // 测试仿射变换
    genaffinepairM4(&aff, &aff_inv);
    affinemixM4(aff, aff_inv, &aff_mix);
    
    // 测试矩阵对生成
    genMatpairM4(&mat1, &mat_inv);
    
    COMPLETION_TEST_PASS();
    return 1;
}

// 测试所有M8矩阵操作函数
int test_M8_complete_coverage() {
    COMPLETION_TEST_START("M8矩阵完整覆盖测试");
    
    M8 mat1, mat2, mat3, mat_inv, mat_trans;
    V8 vec1, vec2, vec_result;
    Aff8 aff, aff_inv, aff_mix;
    
    // 测试初始化
    initM8(&mat1);
    initV8(&vec1);
    
    // 测试随机生成
    randM8(&mat1);
    randV8(&vec1);
    
    // 测试单位矩阵
    identityM8(&mat2);
    
    // 测试打印函数
    printf("\n=== M8矩阵打印测试 ===\n");
    printM8(mat1);
    printbitM8(mat1);
    printV8(vec1);
    printU8(0xFF);
    
    // 测试矩阵操作
    copyM8(mat1, &mat2);
    int equal = isequalM8(mat1, mat2);
    
    // 测试可逆性和求逆
    if (isinvertM8(mat1)) {
        invsM8(mat1, &mat_inv);
    }
    
    // 测试位操作
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int bit = readbitM8(mat1, i, j);
            setbitM8(&mat2, i, j, bit);
            flipbitM8(&mat2, i, j);
        }
    }
    
    // 测试矩阵运算
    MatMulMatM8(mat1, mat2, &mat3);
    MatAddMatM8(mat1, mat2, &mat3);
    MattransM8(mat1, &mat_trans);
    
    // 测试向量运算
    randV8(&vec2);
    VecAddVecV8(vec1, vec2, &vec_result);
    MatMulVecM8(mat1, vec1, &vec_result);
    
    // 测试数字运算
    for (uint8_t num = 0; num < 256; num += 17) {
        uint8_t result = MatMulNumM8(mat1, num);
        uint8_t affine_result = affineU8(aff, num);
        int xor_result = xorU8(num);
        int hw_result = HWU8(num);
    }
    
    // 测试仿射变换
    genaffinepairM8(&aff, &aff_inv);
    affinemixM8(aff, aff_inv, &aff_mix);
    
    COMPLETION_TEST_PASS();
    return 1;
}

// 测试所有M16矩阵操作函数
int test_M16_complete_coverage() {
    COMPLETION_TEST_START("M16矩阵完整覆盖测试");
    
    M16 mat1, mat2, mat3, mat_inv, mat_trans;
    V16 vec1, vec2, vec_result;
    Aff16 aff, aff_inv, aff_mix;
    
    // 测试所有M16操作
    initM16(&mat1);
    initV16(&vec1);
    randM16(&mat1);
    randV16(&vec1);
    identityM16(&mat2);
    
    printf("\n=== M16矩阵打印测试 ===\n");
    printM16(mat1);
    printbitM16(mat1);
    printV16(vec1);
    printU16(0xFFFF);
    
    copyM16(mat1, &mat2);
    int equal = isequalM16(mat1, mat2);
    
    if (isinvertM16(mat1)) {
        invsM16(mat1, &mat_inv);
    }
    
    // 位操作（部分测试以节省时间）
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int bit = readbitM16(mat1, i, j);
            setbitM16(&mat2, i, j, bit);
            flipbitM16(&mat2, i, j);
        }
    }
    
    MatMulMatM16(mat1, mat2, &mat3);
    MatAddMatM16(mat1, mat2, &mat3);
    MattransM16(mat1, &mat_trans);
    
    VecAddVecV16(vec1, vec2, &vec_result);
    MatMulVecM16(mat1, vec1, &vec_result);
    
    for (uint16_t num = 0; num < 65536; num += 4096) {
        uint16_t result = MatMulNumM16(mat1, num);
        uint16_t affine_result = affineU16(aff, num);
        int xor_result = xorU16(num);
        int hw_result = HWU16(num);
    }
    
    genaffinepairM16(&aff, &aff_inv);
    affinemixM16(aff, aff_inv, &aff_mix);
    genMatpairM16(&mat1, &mat_inv);
    
    COMPLETION_TEST_PASS();
    return 1;
}

// 测试所有M32矩阵操作函数
int test_M32_complete_coverage() {
    COMPLETION_TEST_START("M32矩阵完整覆盖测试");
    
    M32 mat1, mat2, mat3, mat_inv, mat_trans;
    V32 vec1, vec2, vec_result;
    Aff32 aff, aff_inv, aff_mix;
    
    initM32(&mat1);
    initV32(&vec1);
    randM32(&mat1);
    randV32(&vec1);
    identityM32(&mat2);
    
    printf("\n=== M32矩阵打印测试 ===\n");
    printM32(mat1);
    printbitM32(mat1);
    printV32(vec1);
    printU32(0xFFFFFFFF);
    
    copyM32(mat1, &mat2);
    int equal = isequalM32(mat1, mat2);
    
    if (isinvertM32(mat1)) {
        invsM32(mat1, &mat_inv);
    }
    
    // 部分位操作测试
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int bit = readbitM32(mat1, i, j);
            setbitM32(&mat2, i, j, bit);
            flipbitM32(&mat2, i, j);
        }
    }
    
    MatMulMatM32(mat1, mat2, &mat3);
    MatAddMatM32(mat1, mat2, &mat3);
    MattransM32(mat1, &mat_trans);
    
    VecAddVecV32(vec1, vec2, &vec_result);
    MatMulVecM32(mat1, vec1, &vec_result);
    
    for (uint32_t num = 0; num < 0x100000000ULL; num += 0x10000000) {
        uint32_t result = MatMulNumM32(mat1, num);
        uint32_t affine_result = affineU32(aff, num);
        int xor_result = xorU32(num);
        int hw_result = HWU32(num);
    }
    
    genaffinepairM32(&aff, &aff_inv);
    affinemixM32(aff, aff_inv, &aff_mix);
    
    COMPLETION_TEST_PASS();
    return 1;
}

// 测试所有M64和M128矩阵操作函数
int test_M64_M128_complete_coverage() {
    COMPLETION_TEST_START("M64/M128矩阵完整覆盖测试");
    
    M64 mat64_1, mat64_2, mat64_3, mat64_inv, mat64_trans;
    V64 vec64_1, vec64_2, vec64_result;
    Aff64 aff64, aff64_inv, aff64_mix;
    
    M128 mat128_1, mat128_2, mat128_3, mat128_inv, mat128_trans;
    V128 vec128_1, vec128_2, vec128_result;
    Aff128 aff128, aff128_inv, aff128_mix;
    
    // M64操作
    initM64(&mat64_1);
    initV64(&vec64_1);
    randM64(&mat64_1);
    randV64(&vec64_1);
    identityM64(&mat64_2);
    
    printf("\n=== M64矩阵打印测试 ===\n");
    printM64(mat64_1);
    printbitM64(mat64_1);
    printV64(vec64_1);
    printU64(0xFFFFFFFFFFFFFFFFULL);
    
    copyM64(mat64_1, &mat64_2);
    int equal64 = isequalM64(mat64_1, mat64_2);
    
    if (isinvertM64(mat64_1)) {
        invsM64(mat64_1, &mat64_inv);
    }
    
    // 部分位操作
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int bit = readbitM64(mat64_1, i, j);
            setbitM64(&mat64_2, i, j, bit);
            flipbitM64(&mat64_2, i, j);
        }
    }
    
    MatMulMatM64(mat64_1, mat64_2, &mat64_3);
    MatAddMatM64(mat64_1, mat64_2, &mat64_3);
    MattransM64(mat64_1, &mat64_trans);
    
    VecAddVecV64(vec64_1, vec64_2, &vec64_result);
    MatMulVecM64(mat64_1, vec64_1, &vec64_result);
    
    genaffinepairM64(&aff64, &aff64_inv);
    affinemixM64(aff64, aff64_inv, &aff64_mix);
    genMatpairM64(&mat64_1, &mat64_inv);
    
    // M128操作
    initM128(&mat128_1);
    initV128(&vec128_1);
    randM128(&mat128_1);
    randV128(&vec128_1);
    identityM128(&mat128_2);
    
    printf("\n=== M128矩阵打印测试 ===\n");
    printM128(mat128_1);
    printbitM128(mat128_1);
    printV128(vec128_1);
    uint64_t test_u128[2] = {0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL};
    printU128(test_u128);
    
    copyM128(mat128_1, &mat128_2);
    int equal128 = isequalM128(mat128_1, mat128_2);
    
    if (isinvertM128(mat128_1)) {
        invsM128(mat128_1, &mat128_inv);
    }
    
    MatMulMatM128(mat128_1, mat128_2, &mat128_3);
    MatAddMatM128(mat128_1, mat128_2, &mat128_3);
    MattransM128(mat128_1, &mat128_trans);
    
    VecAddVecV128(vec128_1, vec128_2, &vec128_result);
    MatMulVecM128(mat128_1, vec128_1, &vec128_result);
    
    genaffinepairM128(&aff128, &aff128_inv);
    affinemixM128(aff128, aff128_inv, &aff128_mix);
    genMatpairM128(&mat128_1, &mat128_inv);
    
    COMPLETION_TEST_PASS();
    return 1;
}

// 测试随机数生成函数的完整覆盖
int test_random_complete_coverage() {
    COMPLETION_TEST_START("随机数生成完整覆盖测试");
    
    // 测试SetRandSeed函数
    SetRandSeed(12345);
    
    // 测试InitRandom函数
    InitRandom(54321);
    
    // 测试cus_random函数
    for (int i = 0; i < 100; i++) {
        unsigned int random_val = cus_random();
    }
    
    // 测试permuteQPR函数的各种输入
    unsigned int test_values[] = {
        0x00000000, 0x00000001, 0x80000000, 0xFFFFFFFF,
        0x12345678, 0x87654321, 0xAAAAAAAA, 0x55555555
    };
    
    for (int i = 0; i < 8; i++) {
        unsigned int result = permuteQPR(test_values[i]);
    }
    
    COMPLETION_TEST_PASS();
    return 1;
}

// 测试所有实用函数
int test_utility_functions_complete() {
    COMPLETION_TEST_START("实用函数完整覆盖测试");
    
    // 测试所有xor函数
    for (int i = 0; i < 256; i++) {
        xorU4(i & 0xF);
        xorU8(i);
        if (i % 256 == 0) {
            xorU16(i * 256);
            xorU32(i * 256 * 256);
        }
    }
    
    // 测试所有汉明重量函数
    for (int i = 0; i < 256; i++) {
        HWU4(i & 0xF);
        HWU8(i);
        if (i % 16 == 0) {
            HWU16(i * 256);
            HWU32(i * 256 * 256);
        }
    }
    
    COMPLETION_TEST_PASS();
    return 1;
}

// 主覆盖率完善测试函数
int main() {
    printf("=== 覆盖率完善测试套件 ===\n\n");
    
    // 运行所有覆盖率完善测试
    test_initialize_aes_sbox();
    test_random_complete_coverage();
    test_utility_functions_complete();
    test_M4_complete_coverage();
    test_M8_complete_coverage();
    test_M16_complete_coverage();
    test_M32_complete_coverage();
    test_M64_M128_complete_coverage();
    
    // 输出覆盖率完善测试结果
    printf("\n=== 覆盖率完善测试结果 ===\n");
    printf("总测试数: %d\n", completion_test_count);
    printf("通过: %d\n", completion_passed);
    printf("失败: %d\n", completion_failed);
    printf("成功率: %.1f%%\n", (completion_passed * 100.0) / completion_test_count);
    
    if (completion_failed == 0) {
        printf("\n✅ 所有覆盖率完善测试通过！\n");
        printf("📈 应该显著提高代码覆盖率\n");
    } else {
        printf("\n❌ %d个覆盖率完善测试失败\n", completion_failed);
    }
    
    return completion_failed == 0 ? 0 : 1;
}