#!/usr/bin/env python3
"""
白盒AES算法代码覆盖率分析脚本
该脚本分析gcov生成的覆盖率数据，并生成详细的覆盖率报告
"""

import os
import re
import sys
from pathlib import Path
import json

class CoverageAnalyzer:
    def __init__(self, build_dir="build_coverage"):
        self.build_dir = build_dir
        self.coverage_data = {}
        self.total_lines = 0
        self.executed_lines = 0
        self.unexecuted_lines = []
        
    def parse_gcov_file(self, gcov_file):
        """解析单个gcov文件"""
        print(f"分析文件: {gcov_file}")
        
        file_data = {
            'filename': gcov_file,
            'total_lines': 0,
            'executed_lines': 0,
            'unexecuted_lines': [],
            'coverage_percent': 0.0
        }
        
        try:
            with open(gcov_file, 'r', encoding='utf-8', errors='ignore') as f:
                lines = f.readlines()
                
            for line_num, line in enumerate(lines, 1):
                line = line.strip()
                
                # 跳过头部信息行
                if line.startswith('file:') or line.startswith('function:') or line.startswith('lcount:'):
                    continue
                
                # 解析代码行
                if ':' in line:
                    parts = line.split(':', 2)
                    if len(parts) >= 3:
                        execution_count = parts[0].strip()
                        source_line_num = parts[1].strip()
                        source_code = parts[2] if len(parts) > 2 else ""
                        
                        # 跳过空行和注释行
                        if not source_code.strip() or source_code.strip().startswith('//') or source_code.strip().startswith('/*'):
                            continue
                            
                        file_data['total_lines'] += 1
                        
                        if execution_count == '#####':
                            # 未执行的行
                            file_data['unexecuted_lines'].append({
                                'line_num': source_line_num,
                                'code': source_code.strip()
                            })
                        elif execution_count.isdigit() and int(execution_count) > 0:
                            # 执行过的行
                            file_data['executed_lines'] += 1
                        elif execution_count == '-':
                            # 不可执行的行（如声明、空行等）
                            file_data['total_lines'] -= 1  # 不计入总数
                            
            # 计算覆盖率
            if file_data['total_lines'] > 0:
                file_data['coverage_percent'] = (file_data['executed_lines'] / file_data['total_lines']) * 100
                
        except Exception as e:
            print(f"解析 {gcov_file} 时出错: {e}")
            return None
            
        return file_data
    
    def analyze_all_gcov_files(self):
        """分析所有gcov文件"""
        gcov_files = list(Path(self.build_dir).glob("*.gcov"))
        
        if not gcov_files:
            print("未找到gcov文件，请确保已运行测试并启用了覆盖率")
            return False
            
        print(f"找到 {len(gcov_files)} 个gcov文件")
        
        for gcov_file in gcov_files:
            file_data = self.parse_gcov_file(gcov_file)
            if file_data:
                # 提取源文件名
                source_name = gcov_file.stem
                if source_name.endswith('.c'):
                    source_name = source_name[:-2]
                elif '#' in source_name:
                    # 处理路径中的#符号
                    source_name = source_name.split('#')[-1]
                    
                self.coverage_data[source_name] = file_data
                self.total_lines += file_data['total_lines']
                self.executed_lines += file_data['executed_lines']
                
        return True
    
    def generate_report(self):
        """生成覆盖率报告"""
        print("\n" + "="*60)
        print("代码覆盖率详细报告")
        print("="*60)
        
        # 总体覆盖率
        overall_coverage = (self.executed_lines / self.total_lines * 100) if self.total_lines > 0 else 0
        print(f"\n总体覆盖率: {self.executed_lines}/{self.total_lines} 行 ({overall_coverage:.2f}%)")
        
        # 按文件分析
        print(f"\n{'文件名':<25} {'总行数':<8} {'执行行数':<8} {'覆盖率':<8} {'状态'}")
        print("-" * 60)
        
        for filename, data in sorted(self.coverage_data.items()):
            status = "✅ 优秀" if data['coverage_percent'] >= 95 else \
                    "⚠️  良好" if data['coverage_percent'] >= 80 else \
                    "❌ 需改进"
            
            print(f"{filename:<25} {data['total_lines']:<8} {data['executed_lines']:<8} {data['coverage_percent']:<7.1f}% {status}")
        
        # 未执行代码分析
        print(f"\n{'='*60}")
        print("未执行代码分析")
        print("="*60)
        
        for filename, data in self.coverage_data.items():
            if data['unexecuted_lines']:
                print(f"\n文件: {filename}")
                print(f"未执行行数: {len(data['unexecuted_lines'])}")
                
                for unexec in data['unexecuted_lines'][:10]:  # 只显示前10行
                    print(f"  行 {unexec['line_num']}: {unexec['code']}")
                
                if len(data['unexecuted_lines']) > 10:
                    print(f"  ... 还有 {len(data['unexecuted_lines']) - 10} 行未执行")
        
        # 覆盖率建议
        print(f"\n{'='*60}")
        print("覆盖率改进建议")
        print("="*60)
        
        if overall_coverage >= 95:
            print("🎉 覆盖率优秀！已达到95%以上")
        elif overall_coverage >= 80:
            print("⚠️  覆盖率良好，建议补充以下测试:")
        else:
            print("❌ 覆盖率需要改进，建议优先补充以下测试:")
        
        # 分析未执行代码的模式
        error_handling_lines = 0
        boundary_check_lines = 0
        loop_lines = 0
        
        for filename, data in self.coverage_data.items():
            for unexec in data['unexecuted_lines']:
                code = unexec['code'].lower()
                if 'if' in code and ('error' in code or 'null' in code or '!=' in code):
                    error_handling_lines += 1
                elif 'if' in code and ('>' in code or '<' in code or '==' in code):
                    boundary_check_lines += 1
                elif 'for' in code or 'while' in code:
                    loop_lines += 1
        
        if error_handling_lines > 0:
            print(f"- 错误处理分支: {error_handling_lines} 行未覆盖")
        if boundary_check_lines > 0:
            print(f"- 边界条件检查: {boundary_check_lines} 行未覆盖")
        if loop_lines > 0:
            print(f"- 循环边界条件: {loop_lines} 行未覆盖")
            
        return overall_coverage
    
    def save_json_report(self, filename="coverage_report.json"):
        """保存JSON格式的覆盖率报告"""
        report = {
            'timestamp': str(time.time()),
            'overall_coverage': (self.executed_lines / self.total_lines * 100) if self.total_lines > 0 else 0,
            'total_lines': self.total_lines,
            'executed_lines': self.executed_lines,
            'files': self.coverage_data
        }
        
        with open(os.path.join(self.build_dir, filename), 'w') as f:
            json.dump(report, f, indent=2)
        
        print(f"\nJSON报告已保存到: {self.build_dir}/{filename}")

def main():
    if len(sys.argv) > 1:
        build_dir = sys.argv[1]
    else:
        build_dir = "build_coverage"
    
    if not os.path.exists(build_dir):
        print(f"错误: 构建目录 {build_dir} 不存在")
        print("请先运行 run_coverage_test.sh 脚本")
        return 1
    
    analyzer = CoverageAnalyzer(build_dir)
    
    if not analyzer.analyze_all_gcov_files():
        return 1
    
    overall_coverage = analyzer.generate_report()
    analyzer.save_json_report()
    
    print(f"\n{'='*60}")
    print("覆盖率分析完成")
    print("="*60)
    
    if overall_coverage >= 100:
        print("🎉 完美！达到100%代码覆盖率")
        return 0
    elif overall_coverage >= 95:
        print("✅ 优秀！覆盖率超过95%")
        return 0
    elif overall_coverage >= 80:
        print("⚠️  良好，但建议提高到95%以上")
        return 0
    else:
        print("❌ 覆盖率不足，需要添加更多测试用例")
        return 1

if __name__ == "__main__":
    import time
    sys.exit(main())