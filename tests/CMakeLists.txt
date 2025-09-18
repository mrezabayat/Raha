find_package(GTest REQUIRED)

add_executable(raha_core_tests
    core/ClockTests.cpp
)

target_link_libraries(raha_core_tests
    PRIVATE
        raha_core
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(raha_core_tests)
