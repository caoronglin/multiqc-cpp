cmake_minimum_required(VERSION 3.20)
project(mqc VERSION 1.0.0 LANGUAGES CXX)

# C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 编译警告选项 (GCC/Clang)
# 提供严格的代码质量检查，帮助发现潜在问题
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall               # 常用警告
        -Wextra             # 额外警告
        -Wpedantic          # 严格标准合规
        -Wshadow            # 变量遮蔽警告
        -Wconversion        # 类型转换警告
        -Wsign-conversion   # 符号转换警告
        -Wno-unused-parameter  # 抑制未使用参数警告（常见于回调函数）
    )
endif()

# 编译优化
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-O3 -march=native)
    endif()
endif()

# 地址 sanitizer (可选)
option(ENABLE_ASAN "Enable Address Sanitizer" OFF)
if(ENABLE_ASAN)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

# 查找依赖
find_package(nlohmann_json CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

# 子目录
add_subdirectory(src/mqc)
add_subdirectory(src/fastp)

# 测试
enable_testing()
add_subdirectory(tests)

# 安装规则
install(TARGETS mqc
    RUNTIME DESTINATION bin
)

# 安装 fastp 可执行文件（如果构建了的话）
if(TARGET fastp_bin)
    install(TARGETS fastp_bin
        RUNTIME DESTINATION bin
    )
endif()

install(DIRECTORY src/mqc/templates/
    DESTINATION share/mqc/templates
)

# 打印配置信息
message(STATUS "")
message(STATUS "MultiQC C++ Configuration Summary")
message(STATUS "=================================")
message(STATUS "  Version:              ${PROJECT_VERSION}")
message(STATUS "  Build type:           ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Compiler:         ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  C++ Standard:         ${CMAKE_CXX_STANDARD}")
message(STATUS "  Address Sanitizer:    ${ENABLE_ASAN}")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "  Warning Flags:        -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion")
endif()
message(STATUS "")
message(STATUS "Build with: cmake --build .")
message(STATUS "Run tests with: ctest --test-dir .")
message(STATUS "")
