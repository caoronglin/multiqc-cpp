# fastp Subdirectory

# Check if fastp should be built
option(BUILD_FASTP "Build fastp FASTQ processor" ON)

if(NOT BUILD_FASTP)
    return()
endif()

# fastp submodule is at project root: ${CMAKE_SOURCE_DIR}/fastp/
set(FASTP_SRC_DIR ${CMAKE_SOURCE_DIR}/fastp/src)

# Check if fastp source files exist
if(NOT EXISTS ${FASTP_SRC_DIR}/main.cpp)
    message(WARNING "fastp source files not found at ${FASTP_SRC_DIR}. Skipping fastp build.")
    return()
endif()

add_library(fastp_lib STATIC
    ${FASTP_SRC_DIR}/simd.cpp
    ${FASTP_SRC_DIR}/stats.cpp
    ${FASTP_SRC_DIR}/filter.cpp
    ${FASTP_SRC_DIR}/filterresult.cpp
    ${FASTP_SRC_DIR}/sequence.cpp
    ${FASTP_SRC_DIR}/read.cpp
    ${FASTP_SRC_DIR}/fastqreader.cpp
    ${FASTP_SRC_DIR}/fastareader.cpp
    ${FASTP_SRC_DIR}/writer.cpp
    ${FASTP_SRC_DIR}/htmlreporter.cpp
    ${FASTP_SRC_DIR}/jsonreporter.cpp
    ${FASTP_SRC_DIR}/adaptertrimmer.cpp
    ${FASTP_SRC_DIR}/basecorrector.cpp
    ${FASTP_SRC_DIR}/duplicate.cpp
    ${FASTP_SRC_DIR}/evaluator.cpp
    ${FASTP_SRC_DIR}/matcher.cpp
    ${FASTP_SRC_DIR}/nucleotidetree.cpp
    ${FASTP_SRC_DIR}/options.cpp
    ${FASTP_SRC_DIR}/overlapanalysis.cpp
    ${FASTP_SRC_DIR}/polyx.cpp
    ${FASTP_SRC_DIR}/threadconfig.cpp
)

target_include_directories(fastp_lib PUBLIC
    ${FASTP_SRC_DIR}
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
    ${FASTP_SRC_DIR}/main.cpp
)

target_link_libraries(fastp_bin PRIVATE
    fastp_lib
)

set_target_properties(fastp_bin PROPERTIES
    OUTPUT_NAME "fastp"
)