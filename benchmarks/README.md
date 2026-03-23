# MultiQC C++ Benchmark

本目录包含性能基准测试脚本和数据集。

## 测试目标

- 对比 MultiQC C++ 与 Python 版本的性能差异
- 测试文件搜索速度
- 测试模块解析速度
- 测试内存使用
- 测试报告生成速度

## 测试数据集

### 小型数据集
- 10 个 FastQC 样本
- 5 个 SAMtools flagstat 文件
- 5 个 Picard metrics 文件

### 中型数据集
- 100 个 FastQC 样本
- 50 个 SAMtools 文件
- 50 个 Picard 文件

### 大型数据集
- 1000 个 FastQC 样本
- 500 个 SAMtools 文件
- 500 个 Picard 文件

## 运行基准测试

```bash
# 编译 Release 版本
cmake --build build/release --config Release

# 运行基准测试
./benchmarks/run_benchmarks.sh

# 查看结果
cat benchmarks/results/benchmark_results.md
```

## 预期性能

| 任务 | Python MultiQC | MultiQC C++ | 目标加速比 |
|------|----------------|-------------|------------|
| 10 FastQC | 0.6s | <0.1s | 6x |
| 100 FastQC | 6.2s | <0.5s | 12x |
| 1000 FastQC | 62s | <3s | 20x |
| 内存 (1000 样本) | 2.1GB | <0.5GB | 4x |

## 性能分析工具

- `time` - 基本时间测量
- `valgrind --tool=massif` - 内存分析
- `perf` - Linux 性能分析工具
- Chrome DevTools - HTML 报告加载性能

## 结果记录

详见 [results/benchmark_results.md](results/benchmark_results.md)
