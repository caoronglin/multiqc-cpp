# fastp Subdirectory

# Check if fastp should be built
option(BUILD_FASTP "Build fastp FASTQ processor" ON)

if(NOT BUILD_FASTP)
    return()
endif()

# Copy fastp source files from original location
# For now, create a stub that will be replaced with actual fastp integration

add_library(fastp_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/simd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/stats.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/filter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/filterresult.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/sequence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/read.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/fastqreader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/fastareader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/writer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/htmlreporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/jsonreporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/adaptertrimmer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/basecorrector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/duplicate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/evaluator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/matcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/nucleotidetree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/options.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/overlapanalysis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/polyx.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/threadconfig.cpp
)

target_include_directories(fastp_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src
)

target_link_libraries(fastp_lib PRIVATE
    z
)

# Add compiler-specific flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(fastp_lib PRIVATE
        -Wno-unused-parameter
        -Wno-sign-compare
    )
endif()

# fastp executable (will be integrated with mqc)
add_executable(fastp_bin
    ${CMAKE_CURRENT_SOURCE_DIR}/../fastp/src/main.cpp
)

target_link_libraries(fastp_bin PRIVATE
    fastp_lib
)

set_target_properties(fastp_bin PROPERTIES
    OUTPUT_NAME "fastp"
)
