#pragma once

#include <atomic>
#include <chrono>

namespace raha::core {

class Clock {
public:
    void start(double start_seconds = 0.0);
    void pause();
    void resume();
    void stop();
    void set_speed(double speed);

    [[nodiscard]] double current_time() const;
    [[nodiscard]] bool running() const { return running_; }
    [[nodiscard]] double speed() const { return speed_.load(); }

private:
    using clock_t = std::chrono::steady_clock;

    clock_t::time_point last_tick_ {clock_t::now()};
    double base_time_seconds_ {0.0};
    std::atomic<double> speed_ {1.0};
    std::atomic<bool> running_ {false};
};

} // namespace raha::core
