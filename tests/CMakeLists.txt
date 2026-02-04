set(TEST_SOURCES
    sql_test.cpp
)

add_executable(tinyp_tests ${TEST_SOURCES})

target_link_libraries(tinyp_tests
    tinyp
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tinyp_tests)
