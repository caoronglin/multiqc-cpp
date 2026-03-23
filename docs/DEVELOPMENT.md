# 开发指南

本文档介绍如何为 MultiQC C++ 项目进行开发，包括代码风格、测试、构建和发布流程。

## 快速开始

### 依赖安装

```bash
# Ubuntu/Debian
sudo apt install cmake ninja-build build-essential libz-dev

# macOS
brew install cmake ninja

# 使用 vcpkg 安装 C++ 依赖
./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install
```

### 构建

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### 运行测试

```bash
ctest --test-dir build -j$(nproc) --output-on-failure
```

---

## 代码质量工具

本项目使用以下工具确保代码质量：

### Clang-Format (代码格式化)

项目使用 Clang-Format 统一代码风格。配置文件：`.clang-format`

**格式化代码：**

```bash
# 格式化所有文件
./scripts/format.sh

# 仅检查格式（CI 使用）
./scripts/format.sh --check
```

**编辑器集成：**

| 编辑器 | 插件 | 设置 |
|--------|------|------|
| VS Code | C/C++ 扩展 | `editor.formatOnSave: true` |
| CLion | 内置 | Settings → Editor → Code Style → ClangFormat |
| Vim/Neovim | vim-clang-format | 自动格式化 |
| Emacs | clang-format.el | 保存时格式化 |

### Clang-Tidy (静态分析)

项目使用 Clang-Tidy 进行静态分析。配置文件：`.clang-tidy`

**运行分析：**

```bash
# 检查问题
./scripts/tidy.sh

# 自动修复
./scripts/tidy.sh --fix
```

### EditorConfig (编辑器配置)

项目使用 `.editorconfig` 统一不同编辑器的基本设置：

- UTF-8 编码
- LF 换行
- 删除尾部空白
- C++: 4 空格缩进
- CMake/YAML/JSON: 2 空格缩进
- Python: 4 空格缩进

**安装插件：**

- VS Code: EditorConfig for VS Code
- JetBrains IDEs: 内置支持
- Vim: editorconfig-vim

---

## 编译警告

项目启用了严格的编译警告选项：

```
-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
```

**警告处理原则：**

1. 所有警告应尽快修复
2. 如有误报，使用 `#pragma GCC diagnostic` 局部禁用
3. 不要全局禁用警告

---

## 添加新模块

### 1. 创建模块目录

```bash
mkdir -p src/mqc/modules/mytool
```

### 2. 创建模块文件

**mytool_module.hpp:**

```cpp
#pragma once

#include "mqc/core/module.hpp"

namespace mqc {

class MyToolModule : public BaseModule {
public:
    MyToolModule();
    void parse(const std::vector<MatchedFile>& files) override;
};

} // namespace mqc
```

**mytool_module.cpp:**

```cpp
#include "mytool_module.hpp"

namespace mqc {

MyToolModule::MyToolModule() 
    : BaseModule("mytool", "MyTool", "Description of my tool") {
    // 定义文件匹配模式
    add_file_pattern("mytool_*.txt");
    add_file_pattern("mytool.log");
}

void MyToolModule::parse(const std::vector<MatchedFile>& files) {
    for (const auto& file : files) {
        // 解析逻辑
    }
}

} // namespace mqc
```

### 3. 注册模块

在 `src/mqc/modules/modules.hpp` 中添加：

```cpp
#include "mytool/mytool_module.hpp"

// 在 get_all_modules() 中
mods.push_back(std::make_unique<MyToolModule>());
```

### 4. 添加测试

```cpp
// tests/unit/test_mytool.cpp
#include <gtest/gtest.h>
#include "mqc/modules/mytool/mytool_module.hpp"

TEST(MyToolModule, Name) {
    mqc::MyToolModule mod;
    EXPECT_EQ(mod.name(), "mytool");
}
```

---

## 测试

### 单元测试

```bash
# 运行所有单元测试
ctest --test-dir build -L unit

# 运行特定测试
ctest --test-dir build -R "FastQC"
```

### 集成测试

```bash
# 运行集成测试
ctest --test-dir build -L integration
```

### 性能基准测试

```bash
./benchmarks/run_benchmarks.sh
```

---

## CI/CD

所有提交都会通过 GitHub Actions 运行：

- ✅ 多平台编译（Ubuntu, macOS, Windows）
- ✅ 多编译器支持（GCC 11+, Clang 14+）
- ✅ 单元测试和集成测试
- ✅ 代码格式化检查
- ✅ 静态分析（clang-tidy, cppcheck）
- ✅ 代码覆盖率报告

---

## 发布流程

1. 更新 `CHANGELOG.md`
2. 更新版本号（CMakeLists.txt）
3. 创建 Git tag：`git tag v1.0.0`
4. 推送 tag：`git push origin v1.0.0`
5. GitHub Actions 自动构建发布制品

---

## 代码风格

### 命名约定

| 类型 | 风格 | 示例 |
|------|------|------|
| 命名空间 | lower_case | `mqc` |
| 类/结构体 | CamelCase | `FastQCModule` |
| 函数 | camelBack | `parseFile()` |
| 变量 | lower_case | `sample_count` |
| 常量 | UPPER_CASE | `MAX_SAMPLES` |
| 成员变量 | lower_case_ | `data_` |

### 注释

使用 Doxygen 风格：

```cpp
/**
 * @brief 解析 FastQC 输出文件
 * @param files 匹配的文件列表
 */
void parse(const std::vector<MatchedFile>& files) override;
```

---

## 常见问题

### Q: clang-format 报错 "unknown key"

确保使用 clang-format 14.0 或更高版本：

```bash
clang-format --version
```

### Q: cmake 找不到依赖

确保 vcpkg 已正确安装：

```bash
./vcpkg/vcpkg integrate install
```

### Q: 测试失败

检查测试输出：

```bash
ctest --test-dir build --output-on-failure
```

---

## 联系方式

- 问题反馈：[GitHub Issues](https://github.com/caoronglin/multiqc-cpp/issues)
- 拉取请求：[GitHub Pull Requests](https://github.com/caoronglin/multiqc-cpp/pulls)