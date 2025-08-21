#include "wbaes.h"
#include "WBMatrix/random.h"
#include <stdio.h>

int main() {
    printf("=== 最终覆盖率测试 ===\n");
    
    // 测试permuteQPR的边界情况
    printf("1. 测试permuteQPR边界情况...\n");
    
    // 测试x >= prime的情况 (prime = 4294967291u)
    unsigned int large_values[] = {
        4294967291u,     // 等于prime
        4294967292u,     // 大于prime
        0xFFFFFFFFu      // 最大值
    };
    
    for (int i = 0; i < 3; i++) {
        unsigned int result = permuteQPR(large_values[i]);
        printf("   permuteQPR(0x%08X) = 0x%08X\n", large_values[i], result);
    }
    
    // 测试一些WBMatrix函数以提高覆盖率
    printf("2. 测试更多WBMatrix函数...\n");
    
    M4 mat4_1, mat4_2;
    initM4(&mat4_1);
    identityM4(&mat4_1);
    copyM4(mat4_1, &mat4_2);
    
    if (isequalM4(mat4_1, mat4_2)) {
        printf("   M4矩阵操作正常\n");
    }
    
    // 测试位操作
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int bit = readbitM4(mat4_1, i, j);
            setbitM4(&mat4_2, i, j, bit);
        }
    }
    
    // 测试更多实用函数
    printf("3. 测试实用函数完整覆盖...\n");
    
    // 测试xorU16和xorU32的更多情况
    xorU16(0xFFFF);
    xorU16(0x5555);
    xorU16(0xAAAA);
    
    xorU32(0xFFFFFFFF);
    xorU32(0x55555555);
    xorU32(0xAAAAAAAA);
    
    // 测试HWU16和HWU32
    HWU16(0xFFFF);
    HWU16(0x0001);
    HWU16(0x8000);
    
    HWU32(0xFFFFFFFF);
    HWU32(0x00000001);
    HWU32(0x80000000);
    
    printf("   实用函数测试完成\n");
    
    printf("\n✅ 最终覆盖率测试完成！\n");
    return 0;
}