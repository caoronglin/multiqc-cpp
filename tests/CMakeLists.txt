# Testing

include(GoogleTest)

# Unit tests
add_executable(mqc_tests_unit
    unit/test_module.cpp
    unit/test_file_search.cpp
)

target_link_libraries(mqc_tests_unit PRIVATE
    mqc_core
    GTest::gtest_main
    GTest::gmock
)

gtest_discover_tests(mqc_tests_unit)

# Integration tests
add_executable(mqc_tests_integration
    integration/test_end_to_end.cpp
    integration/test_module_parsing.cpp
)

target_link_libraries(mqc_tests_integration PRIVATE
    mqc_core
    GTest::gtest_main
)

gtest_discover_tests(mqc_tests_integration)
