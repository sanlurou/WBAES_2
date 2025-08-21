#include "wbaes.h"
#include <stdio.h>
#include <string.h>

int main() {
    printf("=== 快速覆盖率测试 ===\n");
    
    // 1. 测试initialize_aes_sbox
    printf("1. 测试initialize_aes_sbox...\n");
    u8 custom_sbox[256];
    initialize_aes_sbox(custom_sbox);
    printf("   S-Box生成完成\n");
    
    // 2. 测试随机数函数
    printf("2. 测试随机数函数...\n");
    SetRandSeed(12345);
    InitRandom(54321);
    for (int i = 0; i < 10; i++) {
        cus_random();
    }
    permuteQPR(0x12345678);
    printf("   随机数函数测试完成\n");
    
    // 3. 测试基本矩阵操作
    printf("3. 测试基本矩阵操作...\n");
    M4 mat4;
    M8 mat8;
    M16 mat16;
    M32 mat32;
    V4 vec4;
    V8 vec8;
    V16 vec16;
    V32 vec32;
    
    // 初始化
    initM4(&mat4);
    initM8(&mat8);
    initM16(&mat16);
    initM32(&mat32);
    initV4(&vec4);
    initV8(&vec8);
    initV16(&vec16);
    initV32(&vec32);
    
    // 随机生成
    randM4(&mat4);
    randM8(&mat8);
    randV4(&vec4);
    randV8(&vec8);
    
    // 实用函数测试
    for (int i = 0; i < 16; i++) {
        xorU4(i);
        HWU4(i);
    }
    
    for (int i = 0; i < 256; i += 17) {
        xorU8(i);
        HWU8(i);
    }
    
    printf("   矩阵操作测试完成\n");
    
    // 4. 核心算法测试
    printf("4. 测试核心算法...\n");
    u8 key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    u8 plaintext[16] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    u8 wb_output[16], std_output[16];
    
    wbaes_gen(key);
    wbaes_encrypt(plaintext, wb_output);
    aes_128_encrypt(plaintext, key, std_output);
    
    if (memcmp(wb_output, std_output, 16) == 0) {
        printf("   ✅ 核心算法测试通过\n");
    } else {
        printf("   ❌ 核心算法测试失败\n");
        return 1;
    }
    
    printf("\n=== 快速覆盖率测试完成 ===\n");
    return 0;
}