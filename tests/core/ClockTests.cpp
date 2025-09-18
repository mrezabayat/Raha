#include "raha/core/Clock.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

TEST(ClockTests, AdvancesWhenRunning) {
    raha::core::Clock clock;
    clock.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    double elapsed = clock.current_time();
    EXPECT_GT(elapsed, 0.01);
}

TEST(ClockTests, PausesAndResumes) {
    raha::core::Clock clock;
    clock.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    clock.pause();
    double paused_time = clock.current_time();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_DOUBLE_EQ(paused_time, clock.current_time());
    clock.resume();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_GT(clock.current_time(), paused_time);
}
