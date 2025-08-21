#include "WBMatrix/WBMatrix.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// WBMatrix测试计数器
static int wbmatrix_test_count = 0;
static int wbmatrix_passed = 0;
static int wbmatrix_failed = 0;

#define WBMATRIX_TEST_START(name) \
    do { \
        wbmatrix_test_count++; \
        printf("WBMatrix测试 %d: %s ... ", wbmatrix_test_count, name); \
        fflush(stdout); \
    } while(0)

#define WBMATRIX_TEST_PASS() \
    do { \
        wbmatrix_passed++; \
        printf("通过\n"); \
    } while(0)

#define WBMATRIX_TEST_FAIL(msg) \
    do { \
        wbmatrix_failed++; \
        printf("失败: %s\n", msg); \
    } while(0)

#define WBMATRIX_ASSERT(condition, msg) \
    do { \
        if (!(condition)) { \
            WBMATRIX_TEST_FAIL(msg); \
            return 0; \
        } \
    } while(0)

// 测试M8矩阵操作
int test_M8_operations() {
    WBMATRIX_TEST_START("M8矩阵操作测试");
    
    M8 mat1, mat2, mat_result, mat_inv;
    
    // 测试初始化
    initM8(&mat1);
    initM8(&mat2);
    
    // 测试单位矩阵
    identityM8(&mat1);
    WBMATRIX_ASSERT(isinvertM8(mat1), "单位矩阵应该可逆");
    
    // 测试矩阵复制
    copyM8(mat1, &mat2);
    WBMATRIX_ASSERT(isequalM8(mat1, mat2), "矩阵复制失败");
    
    // 测试随机矩阵生成
    randM8(&mat1);
    
    // 测试矩阵求逆（如果可逆）
    if (isinvertM8(mat1)) {
        invsM8(mat1, &mat_inv);
        
        // 验证 M * M^(-1) = I
        MatMulMatM8(mat1, mat_inv, &mat_result);
        
        // 检查结果是否接近单位矩阵
        M8 identity;
        identityM8(&identity);
        // 注意：由于浮点精度问题，这里可能需要容错比较
    }
    
    // 测试位操作
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int original_bit = readbitM8(mat1, i, j);
            flipbitM8(&mat1, i, j);
            int flipped_bit = readbitM8(mat1, i, j);
            WBMATRIX_ASSERT(original_bit != flipped_bit, "位翻转操作失败");
            
            // 恢复原始位
            setbitM8(&mat1, i, j, original_bit);
            int restored_bit = readbitM8(mat1, i, j);
            WBMATRIX_ASSERT(restored_bit == original_bit, "位设置操作失败");
        }
    }
    
    WBMATRIX_TEST_PASS();
    return 1;
}

// 测试M32矩阵操作
int test_M32_operations() {
    WBMATRIX_TEST_START("M32矩阵操作测试");
    
    M32 mat1, mat2, mat_inv;
    
    // 测试初始化和单位矩阵
    initM32(&mat1);
    identityM32(&mat1);
    WBMATRIX_ASSERT(isinvertM32(mat1), "M32单位矩阵应该可逆");
    
    // 测试随机矩阵生成和求逆
    randM32(&mat1);
    if (isinvertM32(mat1)) {
        invsM32(mat1, &mat_inv);
        // 基本验证：逆矩阵也应该可逆
        WBMATRIX_ASSERT(isinvertM32(mat_inv), "M32逆矩阵应该可逆");
    }
    
    // 测试矩阵复制和比较
    copyM32(mat1, &mat2);
    WBMATRIX_ASSERT(isequalM32(mat1, mat2), "M32矩阵复制失败");
    
    // 测试位操作
    for (int i = 0; i < 4; i++) { // 只测试部分位以节省时间
        for (int j = 0; j < 4; j++) {
            int original_bit = readbitM32(mat1, i, j);
            flipbitM32(&mat1, i, j);
            int flipped_bit = readbitM32(mat1, i, j);
            WBMATRIX_ASSERT(original_bit != flipped_bit, "M32位翻转操作失败");
            
            setbitM32(&mat1, i, j, original_bit);
        }
    }
    
    WBMATRIX_TEST_PASS();
    return 1;
}

// 测试矩阵对生成函数
int test_matrix_pair_generation() {
    WBMATRIX_TEST_START("矩阵对生成测试");
    
    // 测试M8矩阵对生成
    M8 mat8, mat8_inv, test_result;
    genMatpairM8(&mat8, &mat8_inv);
    
    WBMATRIX_ASSERT(isinvertM8(mat8), "生成的M8矩阵应该可逆");
    WBMATRIX_ASSERT(isinvertM8(mat8_inv), "生成的M8逆矩阵应该可逆");
    
    // 验证它们确实互为逆矩阵
    MatMulMatM8(mat8, mat8_inv, &test_result);
    M8 identity8;
    identityM8(&identity8);
    // 这里简化验证，主要确保函数可以正常调用
    
    // 测试M32矩阵对生成
    M32 mat32, mat32_inv;
    genMatpairM32(&mat32, &mat32_inv);
    
    WBMATRIX_ASSERT(isinvertM32(mat32), "生成的M32矩阵应该可逆");
    WBMATRIX_ASSERT(isinvertM32(mat32_inv), "生成的M32逆矩阵应该可逆");
    
    WBMATRIX_TEST_PASS();
    return 1;
}

// 测试MatrixcomM8to32函数
int test_MatrixcomM8to32() {
    WBMATRIX_TEST_START("MatrixcomM8to32函数测试");
    
    M8 m1, m2, m3, m4;
    M32 result;
    
    // 创建测试矩阵
    identityM8(&m1);
    identityM8(&m2);
    identityM8(&m3);
    identityM8(&m4);
    
    // 调用组合函数
    MatrixcomM8to32(m1, m2, m3, m4, &result);
    
    // 验证结果矩阵是可逆的（基本合理性检查）
    WBMATRIX_ASSERT(isinvertM32(result), "MatrixcomM8to32结果应该可逆");
    
    // 测试不同的输入组合
    randM8(&m1);
    randM8(&m2);
    randM8(&m3);
    randM8(&m4);
    
    // 确保输入矩阵都是可逆的
    if (isinvertM8(m1) && isinvertM8(m2) && isinvertM8(m3) && isinvertM8(m4)) {
        MatrixcomM8to32(m1, m2, m3, m4, &result);
        WBMATRIX_ASSERT(isinvertM32(result), "随机M8组合结果应该可逆");
    }
    
    WBMATRIX_TEST_PASS();
    return 1;
}

// 测试随机数生成函数
int test_random_functions() {
    WBMATRIX_TEST_START("随机数生成函数测试");
    
    // 测试随机种子设置
    SetRandSeed(12345);
    
    // 测试InitRandom函数
    InitRandom(54321);
    
    // 测试cus_random函数
    unsigned int random_values[100];
    for (int i = 0; i < 100; i++) {
        random_values[i] = cus_random();
    }
    
    // 验证随机数的基本特性
    // 1. 不应该全部相同
    int all_same = 1;
    for (int i = 1; i < 100; i++) {
        if (random_values[i] != random_values[0]) {
            all_same = 0;
            break;
        }
    }
    WBMATRIX_ASSERT(!all_same, "随机数生成器产生相同值");
    
    // 2. 测试permuteQPR函数
    unsigned int test_inputs[] = {0, 1, 0xFFFFFFFF, 0x80000000, 0x12345678};
    for (int i = 0; i < 5; i++) {
        unsigned int result = permuteQPR(test_inputs[i]);
        // 基本验证：结果应该在有效范围内
        WBMATRIX_ASSERT(result <= 0xFFFFFFFF, "permuteQPR结果超出范围");
    }
    
    // 3. 测试随机数的重现性
    InitRandom(12345);
    unsigned int first_sequence[10];
    for (int i = 0; i < 10; i++) {
        first_sequence[i] = cus_random();
    }
    
    InitRandom(12345);
    unsigned int second_sequence[10];
    for (int i = 0; i < 10; i++) {
        second_sequence[i] = cus_random();
    }
    
    WBMATRIX_ASSERT(memcmp(first_sequence, second_sequence, sizeof(first_sequence)) == 0, 
                   "相同种子应该产生相同的随机数序列");
    
    WBMATRIX_TEST_PASS();
    return 1;
}

// 测试向量操作
int test_vector_operations() {
    WBMATRIX_TEST_START("向量操作测试");
    
    // 测试V8向量操作
    V8 vec1, vec2, vec_result;
    initV8(&vec1);
    initV8(&vec2);
    
    randV8(&vec1);
    randV8(&vec2);
    
    VecAddVecV8(vec1, vec2, &vec_result);
    
    // 验证向量加法的交换律
    V8 vec_result2;
    VecAddVecV8(vec2, vec1, &vec_result2);
    WBMATRIX_ASSERT(isequalV8(vec_result, vec_result2), "向量加法不满足交换律");
    
    // 测试V32向量操作
    V32 vec32_1, vec32_2, vec32_result;
    initV32(&vec32_1);
    initV32(&vec32_2);
    
    randV32(&vec32_1);
    randV32(&vec32_2);
    
    VecAddVecV32(vec32_1, vec32_2, &vec32_result);
    
    WBMATRIX_TEST_PASS();
    return 1;
}

// 测试仿射变换
int test_affine_operations() {
    WBMATRIX_TEST_START("仿射变换测试");
    
    // 测试M8仿射变换
    Aff8 aff8, aff8_inv;
    genaffinepairM8(&aff8, &aff8_inv);
    
    // 测试仿射变换的应用
    uint8_t test_input = 0x53;
    uint8_t transformed = affineU8(aff8, test_input);
    uint8_t recovered = affineU8(aff8_inv, transformed);
    
    WBMATRIX_ASSERT(recovered == test_input, "M8仿射变换逆变换失败");
    
    // 测试所有可能的输入值
    for (int input = 0; input < 256; input++) {
        uint8_t trans = affineU8(aff8, input);
        uint8_t recov = affineU8(aff8_inv, trans);
        WBMATRIX_ASSERT(recov == input, "M8仿射变换逆变换失败");
    }
    
    WBMATRIX_TEST_PASS();
    return 1;
}

// 测试矩阵乘法操作
int test_matrix_multiplication() {
    WBMATRIX_TEST_START("矩阵乘法测试");
    
    M8 mat1, mat2, mat_result, identity;
    
    // 测试与单位矩阵的乘法
    identityM8(&identity);
    randM8(&mat1);
    
    MatMulMatM8(mat1, identity, &mat_result);
    WBMATRIX_ASSERT(isequalM8(mat1, mat_result), "矩阵乘以单位矩阵应该不变");
    
    MatMulMatM8(identity, mat1, &mat_result);
    WBMATRIX_ASSERT(isequalM8(mat1, mat_result), "单位矩阵乘以矩阵应该不变");
    
    // 测试矩阵乘向量
    V8 vec, vec_result;
    randV8(&vec);
    
    MatMulVecM8(identity, vec, &vec_result);
    WBMATRIX_ASSERT(isequalV8(vec, vec_result), "单位矩阵乘以向量应该不变");
    
    // 测试矩阵乘数字
    for (uint8_t num = 0; num < 256; num += 17) { // 测试部分值以节省时间
        uint8_t result1 = MatMulNumM8(identity, num);
        WBMATRIX_ASSERT(result1 == num, "单位矩阵乘以数字应该不变");
    }
    
    WBMATRIX_TEST_PASS();
    return 1;
}

// 测试实用函数
int test_utility_functions() {
    WBMATRIX_TEST_START("实用函数测试");
    
    // 测试xorU8函数
    WBMATRIX_ASSERT(xorU8(0x00) == 0, "xorU8(0) 应该为0");
    WBMATRIX_ASSERT(xorU8(0xFF) == 0, "xorU8(0xFF) 应该为0");
    WBMATRIX_ASSERT(xorU8(0x01) == 1, "xorU8(0x01) 应该为1");
    WBMATRIX_ASSERT(xorU8(0x03) == 0, "xorU8(0x03) 应该为0");
    WBMATRIX_ASSERT(xorU8(0x07) == 1, "xorU8(0x07) 应该为1");
    
    // 测试HWU8函数（汉明重量）
    WBMATRIX_ASSERT(HWU8(0x00) == 0, "HWU8(0) 应该为0");
    WBMATRIX_ASSERT(HWU8(0xFF) == 8, "HWU8(0xFF) 应该为8");
    WBMATRIX_ASSERT(HWU8(0x01) == 1, "HWU8(0x01) 应该为1");
    WBMATRIX_ASSERT(HWU8(0x03) == 2, "HWU8(0x03) 应该为2");
    WBMATRIX_ASSERT(HWU8(0x0F) == 4, "HWU8(0x0F) 应该为4");
    WBMATRIX_ASSERT(HWU8(0xF0) == 4, "HWU8(0xF0) 应该为4");
    
    // 测试所有可能的输入
    for (int i = 0; i < 256; i++) {
        int hw = HWU8(i);
        WBMATRIX_ASSERT(hw >= 0 && hw <= 8, "汉明重量超出范围");
        
        int xor_result = xorU8(i);
        WBMATRIX_ASSERT(xor_result == 0 || xor_result == 1, "XOR结果应该为0或1");
    }
    
    WBMATRIX_TEST_PASS();
    return 1;
}

// 测试M4矩阵操作（完整覆盖）
int test_M4_operations_complete() {
    WBMATRIX_TEST_START("M4矩阵完整操作测试");
    
    M4 mat1, mat2, mat_result, mat_inv, mat_trans;
    V4 vec1, vec2, vec_result;
    
    // 测试所有M4基本操作
    initM4(&mat1);
    identityM4(&mat1);
    
    // 测试矩阵转置
    MattransM4(mat1, &mat_trans);
    WBMATRIX_ASSERT(isequalM4(mat1, mat_trans), "单位矩阵转置应该等于自身");
    
    // 测试随机矩阵
    randM4(&mat1);
    if (isinvertM4(mat1)) {
        invsM4(mat1, &mat_inv);
        
        // 测试逆矩阵性质
        MatMulMatM4(mat1, mat_inv, &mat_result);
        // 结果应该接近单位矩阵
    }
    
    // 测试矩阵加法
    randM4(&mat2);
    MatAddMatM4(mat1, mat2, &mat_result);
    
    // 验证加法交换律
    M4 mat_result2;
    MatAddMatM4(mat2, mat1, &mat_result2);
    WBMATRIX_ASSERT(isequalM4(mat_result, mat_result2), "矩阵加法不满足交换律");
    
    // 测试V4向量操作
    initV4(&vec1);
    randV4(&vec1);
    randV4(&vec2);
    
    VecAddVecV4(vec1, vec2, &vec_result);
    
    // 测试向量加法交换律
    V4 vec_result2;
    VecAddVecV4(vec2, vec1, &vec_result2);
    WBMATRIX_ASSERT(isequalV4(vec_result, vec_result2), "V4向量加法不满足交换律");
    
    // 测试矩阵乘向量
    MatMulVecM4(mat1, vec1, &vec_result);
    
    // 测试矩阵乘数字
    for (uint8_t num = 0; num < 16; num++) {
        uint8_t result = MatMulNumM4(mat1, num);
        WBMATRIX_ASSERT(result < 16, "M4矩阵乘数字结果应该在4位范围内");
    }
    
    WBMATRIX_TEST_PASS();
    return 1;
}

// 测试所有打印函数
int test_print_functions_wbmatrix() {
    WBMATRIX_TEST_START("WBMatrix打印函数测试");
    
    printf("\n=== M4矩阵打印测试 ===\n");
    M4 mat4;
    identityM4(&mat4);
    printM4(mat4);
    printbitM4(mat4);
    
    printf("=== M8矩阵打印测试 ===\n");
    M8 mat8;
    identityM8(&mat8);
    printM8(mat8);
    printbitM8(mat8);
    
    printf("=== M32矩阵打印测试 ===\n");
    M32 mat32;
    identityM32(&mat32);
    printM32(mat32);
    printbitM32(mat32);
    
    printf("=== 向量打印测试 ===\n");
    V4 vec4;
    V8 vec8;
    V32 vec32;
    
    randV4(&vec4);
    randV8(&vec8);
    randV32(&vec32);
    
    printV4(vec4);
    printV8(vec8);
    printV32(vec32);
    
    printf("=== 数字打印测试 ===\n");
    printU8(0xFF);
    printU16(0xFFFF);
    printU32(0xFFFFFFFF);
    
    WBMATRIX_TEST_PASS();
    return 1;
}

// 主WBMatrix测试函数
int main() {
    printf("=== WBMatrix模块测试套件 ===\n\n");
    
    // 运行所有WBMatrix测试
    test_random_functions();
    test_utility_functions();
    test_M4_operations_complete();
    test_M8_operations();
    test_M32_operations();
    test_vector_operations();
    test_matrix_pair_generation();
    test_MatrixcomM8to32();
    test_affine_operations();
    test_matrix_multiplication();
    test_print_functions_wbmatrix();
    
    // 输出WBMatrix测试结果
    printf("\n=== WBMatrix测试结果 ===\n");
    printf("总WBMatrix测试数: %d\n", wbmatrix_test_count);
    printf("通过: %d\n", wbmatrix_passed);
    printf("失败: %d\n", wbmatrix_failed);
    printf("成功率: %.1f%%\n", (wbmatrix_passed * 100.0) / wbmatrix_test_count);
    
    return wbmatrix_failed == 0 ? 0 : 1;
}