# MultiQC C++ Core Library

add_library(mqc STATIC)

target_sources(mqc PRIVATE
    # Core
    core/module.cpp
)

target_include_directories(mqc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(mqc PUBLIC
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    CLI11::CLI11
    yaml-cpp
)

# Inja (header-only, vendored)
target_include_directories(mqc PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/inja/include
)

# C++20 features
target_compile_features(mqc PUBLIC cxx_std_20)

# Platform-specific
if(WIN32)
    target_compile_definitions(mqc PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
endif()
