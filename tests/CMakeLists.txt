find_package(GTest REQUIRED)

add_executable(vplayer_core_tests
    core/ClockTests.cpp
)

target_link_libraries(vplayer_core_tests
    PRIVATE
        vplayer_core
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(vplayer_core_tests)
