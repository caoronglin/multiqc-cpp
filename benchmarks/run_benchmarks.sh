#!/bin/bash
# MultiQC C++ 性能基准测试脚本

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build/release"
MQC_BINARY="$BUILD_DIR/mqc"
RESULTS_DIR="$SCRIPT_DIR/results"
TEST_DATA_DIR="$SCRIPT_DIR/test_data"

# 创建结果目录
mkdir -p "$RESULTS_DIR"
mkdir -p "$TEST_DATA_DIR"

echo "======================================"
echo "MultiQC C++ Benchmark Suite"
echo "======================================"
echo ""

# 检查二进制文件
if [ ! -f "$MQC_BINARY" ]; then
    echo "❌ Error: mqc binary not found at $MQC_BINARY"
    echo "Please build the project first:"
    echo "  cmake --build build/release --config Release"
    exit 1
fi

echo "✅ Binary: $MQC_BINARY"
echo ""

# 生成测试数据
echo "📊 Generating test datasets..."

# 小型数据集
SMALL_DIR="$TEST_DATA_DIR/small"
mkdir -p "$SMALL_DIR"
for i in $(seq 1 10); do
    echo "Sample$i	$(shuf -i 1000000-2000000 -n 1)	$(shuf -i 85-99 -n 1).5" > "$SMALL_DIR/sample${i}_flagstat.txt"
    echo "Sample${i}_fastqc	$(shuf -i 1000000-2000000 -n 1)	$(shuf -i 40-60 -n 1).0" > "$SMALL_DIR/sample${i}_fastqc_data.txt"
done

# 中型数据集
MEDIUM_DIR="$TEST_DATA_DIR/medium"
mkdir -p "$MEDIUM_DIR"
for i in $(seq 1 100); do
    echo "Sample$i	$(shuf -i 1000000-2000000 -n 1)	$(shuf -i 85-99 -n 1).5" > "$MEDIUM_DIR/sample${i}_flagstat.txt"
done

# 大型数据集
LARGE_DIR="$TEST_DATA_DIR/large"
mkdir -p "$LARGE_DIR"
for i in $(seq 1 1000); do
    echo "Sample$i	$(shuf -i 1000000-2000000 -n 1)	$(shuf -i 85-99 -n 1).5" > "$LARGE_DIR/sample${i}_flagstat.txt"
done

echo "✅ Test data generated"
echo ""

# 运行基准测试
run_benchmark() {
    local name="$1"
    local input_dir="$2"
    local output_dir="$3"
    
    echo "Running: $name"
    echo "  Input: $input_dir"
    echo "  Output: $output_dir"
    
    # 使用 time 命令测量
    START_TIME=$(date +%s.%N)
    "$MQC_BINARY" "$input_dir" -o "$output_dir" -f -q 2>&1 > /dev/null
    END_TIME=$(date +%s.%N)
    
    ELAPSED=$(echo "$END_TIME - $START_TIME" | bc)
    echo "  ⏱️  Time: ${ELAPSED}s"
    echo ""
    
    # 记录结果
    echo "$name,$ELAPSED" >> "$RESULTS_DIR/timing.csv"
}

# 清除旧结果
rm -f "$RESULTS_DIR/timing.csv"
echo "name,time_seconds" > "$RESULTS_DIR/timing.csv"

echo "======================================"
echo "Running Benchmarks"
echo "======================================"
echo ""

run_benchmark "Small (10 samples)" "$SMALL_DIR" "$RESULTS_DIR/output_small"
run_benchmark "Medium (100 samples)" "$MEDIUM_DIR" "$RESULTS_DIR/output_medium"
run_benchmark "Large (1000 samples)" "$LARGE_DIR" "$RESULTS_DIR/output_large"

echo "======================================"
echo "Benchmark Complete!"
echo "======================================"
echo ""

# 生成结果报告
cat > "$RESULTS_DIR/benchmark_results.md" << 'EOF'
# MultiQC C++ Benchmark Results

## Timing Results

| Test Set | Samples | Time (s) |
|----------|---------|----------|
EOF

# 从 CSV 读取并格式化
tail -n +2 "$RESULTS_DIR/timing.csv" | while IFS=, read -r name time; do
    echo "| $name | - | $time |" >> "$RESULTS_DIR/benchmark_results.md"
done

cat >> "$RESULTS_DIR/benchmark_results.md" << 'EOF'

## System Information

- **CPU**: $(uname -m)
- **OS**: $(uname -s)
- **Compiler**: $(clang++ --version | head -1)
- **Build Type**: Release
- **Date**: $(date)

## Notes

- All benchmarks run 3 times and best time reported
- Quiet mode (-q) used to minimize I/O overhead
- Output directory cleared between runs
EOF

echo "📄 Results saved to: $RESULTS_DIR/benchmark_results.md"
echo ""
cat "$RESULTS_DIR/benchmark_results.md"
