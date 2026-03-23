# GitHub Actions CI/CD

本项目使用 GitHub Actions 进行自动化测试和部署。

## 工作流程

### CI/CD Pipeline (`.github/workflows/ci-cd.yml`)

#### 1. **Build & Test** 
- **平台**: Ubuntu 22.04/24.04, macOS 13/14
- **编译器**: GCC 11, Clang 14
- **功能**:
  - 使用 ccache 加速编译
  - 运行单元测试
  - 上传构建产物和测试结果

#### 2. **Code Quality**
- **静态分析**: clang-tidy, cppcheck
- **测试覆盖**: lcov + Codecov
- **功能**:
  - 代码质量检查
  - 覆盖率报告
  - 上传分析结果

#### 3. **Performance Benchmarks**
- **运行基准测试**: `benchmarks/run_benchmarks.sh`
- **功能**:
  - 性能测试（小/中/大数据集）
  - 自动生成 Markdown 报告
  - PR 评论中显示结果

#### 4. **Documentation**
- **生成**: Doxygen 文档
- **部署**: GitHub Pages
- **功能**:
  - API 文档生成
  - 自动部署到 gh-pages

#### 5. **Release**
- **触发**: 发布 GitHub Release
- **功能**:
  - 构建 Release 版本
  - 创建 tarball
  - 上传到 Release assets

#### 6. **Deploy Pages**
- **触发**: main 分支推送
- **功能**:
  - 部署文档到 GitHub Pages
  - 自动生成站点

## 触发条件

- **Push**: main, master, develop 分支
- **Pull Request**: 所有 PR
- **Release**: 发布新版本

## 配置选项

在 CI 环境中可以设置以下 CMake 选项：

```bash
# 启用测试
-DCMAKE_BUILD_TYPE=Debug
-DENABLE_TESTING=ON

# 启用 clang-tidy
-DENABLE_CLANG_TIDY=ON

# 启用覆盖率
-DENABLE_COVERAGE=ON

# 启用 Address Sanitizer
-DENABLE_ASAN=ON
```

## 查看结果

### 构建状态
访问：https://github.com/caoronglin/multiqc-cpp/actions

### 测试覆盖率
访问：https://codecov.io/gh/caoronglin/multiqc-cpp

### 文档
访问：https://caoronglin.github.io/multiqc-cpp/

### Benchmark 结果
在 PR 评论中自动显示，或查看 Artifacts。

## 本地测试 CI 配置

可以使用 [act](https://github.com/nektos/act) 在本地运行 GitHub Actions：

```bash
# 安装 act
brew install act

# 运行所有工作流
act

# 运行特定工作流
act push
act pull_request

# 使用不同的 runner
act -P ubuntu-latest=catthehacker/ubuntu:act-latest
```

## 添加新的测试

### 1. 单元测试
在 `tests/unit/` 添加测试文件，CMake 会自动发现。

### 2. 基准测试
在 `benchmarks/` 添加测试脚本。

### 3. 集成测试
在 `tests/integration/` 添加端到端测试。

## 故障排除

### 构建失败
1. 检查 Actions 日志
2. 下载 artifacts 查看详细错误
3. 本地复现问题

### 测试失败
1. 查看 CTest 输出
2. 下载测试日志
3. 本地运行 `ctest --output-on-failure`

### 覆盖率低
1. 查看 Codecov 报告
2. 添加缺失的测试用例
3. 确保测试覆盖所有代码路径

## 贡献指南

1. Fork 仓库
2. 创建功能分支
3. 编写测试
4. 确保 CI 通过
5. 提交 PR

CI 会自动运行所有检查，确保代码质量。
