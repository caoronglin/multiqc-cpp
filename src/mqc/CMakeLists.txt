# MultiQC C++ Core Library and Executable

# Core library (static)
add_library(mqc_core STATIC)

target_sources(mqc_core PRIVATE
    # Core
    core/module.cpp
    core/file_search.cpp
    core/report.cpp
    core/config.cpp
    
    # Modules (Tier 1)
    modules/fastqc/fastqc_module.cpp
    modules/samtools/samtools_module.cpp
    modules/picard/picard_module.cpp
    modules/bowtie2/bowtie2_module.cpp
    modules/star/star_module.cpp
    modules/salmon/salmon_module.cpp
    modules/cutadapt/cutadapt_module.cpp
    modules/bismark/bismark_module.cpp
    modules/bcftools/bcftools_module.cpp
    modules/featurecounts/featurecounts_module.cpp
    modules/bclconvert/bclconvert_module.cpp
)

target_include_directories(mqc_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(mqc_core PUBLIC
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    CLI11::CLI11
    yaml-cpp::yaml-cpp
)

# Inja (header-only, vendored)
target_include_directories(mqc_core PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/inja/include
)

# C++20 features
target_compile_features(mqc_core PUBLIC cxx_std_20)

# Platform-specific
if(WIN32)
    target_compile_definitions(mqc_core PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
endif()

# Main executable
add_executable(mqc
    cli.cpp
    main.cpp
)

target_link_libraries(mqc PRIVATE
    mqc_core
)
