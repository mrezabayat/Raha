#include "vplayer/utils/ThreadPool.hpp"

#include <chrono>

namespace vplayer::utils {

ThreadPool::ThreadPool(std::size_t thread_count) {
    if (thread_count == 0) {
        thread_count = 1;
    }
    workers_.reserve(thread_count);
    for (std::size_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back([this] { worker_loop(); });
    }
}

ThreadPool::~ThreadPool() {
    running_ = false;
    queue_.stop();
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::wait_idle() {
    using namespace std::chrono_literals;
    while (running_ && !queue_.empty()) {
        std::this_thread::sleep_for(10ms);
    }
}

void ThreadPool::worker_loop() {
    while (running_) {
        auto task = queue_.wait_and_pop();
        if (!task) {
            if (!running_) {
                return;
            }
            continue;
        }
        task();
    }
}

} // namespace vplayer::utils
