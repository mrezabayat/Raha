#include "vplayer/core/Clock.hpp"

namespace vplayer::core {

void Clock::start(double start_seconds) {
    base_time_seconds_ = start_seconds;
    last_tick_ = clock_t::now();
    running_ = true;
}

void Clock::pause() {
    if (!running_) {
        return;
    }
    base_time_seconds_ = current_time();
    running_ = false;
}

void Clock::resume() {
    if (running_) {
        return;
    }
    last_tick_ = clock_t::now();
    running_ = true;
}

void Clock::stop() {
    running_ = false;
    base_time_seconds_ = 0.0;
}

void Clock::set_speed(double speed) {
    if (speed <= 0.0) {
        speed = 1.0;
    }
    if (running_) {
        base_time_seconds_ = current_time();
        last_tick_ = clock_t::now();
    }
    speed_.store(speed);
}

double Clock::current_time() const {
    if (!running_) {
        return base_time_seconds_;
    }
    auto now = clock_t::now();
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(now - last_tick_).count();
    double seconds = static_cast<double>(diff) / 1'000'000.0;
    return base_time_seconds_ + seconds * speed_.load();
}

} // namespace vplayer::core
