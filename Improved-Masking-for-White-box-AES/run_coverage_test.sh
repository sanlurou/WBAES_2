#!/bin/bash

# 白盒AES算法覆盖率测试脚本
# 该脚本将编译测试程序，运行所有测试，并生成覆盖率报告

echo "=== 白盒AES算法覆盖率测试脚本 ==="
echo

# 检查必要的工具
echo "检查必要工具..."
if ! command -v gcc &> /dev/null; then
    echo "错误: 未找到gcc编译器"
    exit 1
fi

if ! command -v gcov &> /dev/null; then
    echo "错误: 未找到gcov工具"
    exit 1
fi

if ! command -v lcov &> /dev/null; then
    echo "警告: 未找到lcov工具，将使用gcov生成基本报告"
    USE_LCOV=false
else
    USE_LCOV=true
fi

# 清理之前的构建
echo "清理之前的构建文件..."
rm -rf build_coverage
mkdir -p build_coverage
cd build_coverage

# 使用覆盖率配置编译
echo "配置CMake（启用覆盖率）..."
cp ../CMakeLists_coverage.txt ../CMakeLists.txt
cmake .. -DCMAKE_BUILD_TYPE=Debug

echo "编译项目..."
make clean
make

if [ $? -ne 0 ]; then
    echo "编译失败！"
    exit 1
fi

echo "编译成功！"
echo

# 运行所有测试
echo "=== 运行测试套件 ==="

echo "1. 运行原始测试..."
./WBAES
echo

echo "2. 运行综合测试..."
./WBAES_COMPREHENSIVE_TEST
echo

echo "3. 运行单元测试..."
./WBAES_UNIT_TEST
echo

echo "4. 运行边界测试..."
./WBAES_BOUNDARY_TEST
echo

echo "5. 运行稳定性测试..."
./WBAES_STABILITY_TEST
echo

echo "6. 运行WBMatrix模块测试..."
./WBAES_WBMATRIX_TEST
echo

echo "7. 运行主测试套件..."
./WBAES_MASTER_TEST
echo

# 生成覆盖率报告
echo "=== 生成覆盖率报告 ==="

# 查找所有gcda文件
echo "查找覆盖率数据文件..."
find . -name "*.gcda" -ls

# 使用gcov生成基本报告
echo "生成gcov报告..."
gcov ../src/*.c ../src/WBMatrix/*.c

# 如果有lcov，生成HTML报告
if [ "$USE_LCOV" = true ]; then
    echo "生成lcov HTML报告..."
    lcov --capture --directory . --output-file coverage.info
    lcov --remove coverage.info '/usr/*' --output-file coverage_filtered.info
    genhtml coverage_filtered.info --output-directory coverage_html
    
    echo "HTML覆盖率报告已生成在: coverage_html/index.html"
fi

# 分析覆盖率数据
echo
echo "=== 覆盖率分析 ==="

# 统计每个源文件的覆盖率
for gcov_file in *.gcov; do
    if [ -f "$gcov_file" ]; then
        echo "分析 $gcov_file ..."
        
        # 提取文件名
        filename=$(echo "$gcov_file" | sed 's/\.gcov$//')
        
        # 统计行数
        total_lines=$(grep -c ":" "$gcov_file" | head -1)
        executed_lines=$(grep -c "^[[:space:]]*[1-9]" "$gcov_file" | head -1)
        unexecuted_lines=$(grep -c "^[[:space:]]*#####" "$gcov_file" | head -1)
        
        if [ "$total_lines" -gt 0 ]; then
            coverage_percent=$(echo "scale=2; $executed_lines * 100 / $total_lines" | bc -l 2>/dev/null || echo "0")
            echo "  $filename: $executed_lines/$total_lines 行被执行 (${coverage_percent}%)"
            
            # 显示未执行的行
            if [ "$unexecuted_lines" -gt 0 ]; then
                echo "  未执行的行:"
                grep -n "^[[:space:]]*#####" "$gcov_file" | head -10
            fi
        fi
        echo
    fi
done

# 生成总结报告
echo "=== 覆盖率总结 ==="
echo "所有测试已完成。"
echo "详细的覆盖率数据请查看 *.gcov 文件"
if [ "$USE_LCOV" = true ]; then
    echo "可视化报告请打开: coverage_html/index.html"
fi

echo
echo "=== 建议 ==="
echo "1. 检查 *.gcov 文件中标记为 ##### 的未执行行"
echo "2. 为未覆盖的代码路径添加额外的测试用例"
echo "3. 特别关注错误处理和边界条件"
echo "4. 验证所有函数的返回路径都被测试"

# 运行Python覆盖率分析
echo "=== Python覆盖率分析 ==="
cd ..
if command -v python3 &> /dev/null; then
    echo "运行详细覆盖率分析..."
    python3 analyze_coverage.py build_coverage
else
    echo "未找到python3，跳过详细分析"
fi

echo
echo "=== 覆盖率测试完成 ==="
echo "请查看以下文件获取详细结果:"
echo "- build_coverage/*.gcov - gcov原始报告"
if [ "$USE_LCOV" = true ]; then
    echo "- build_coverage/coverage_html/index.html - HTML可视化报告"
fi
echo "- build_coverage/coverage_report.json - JSON格式报告"
echo
echo "如果覆盖率未达到100%，请:"
echo "1. 查看未执行的代码行"
echo "2. 添加相应的测试用例"
echo "3. 重新运行此脚本验证改进"