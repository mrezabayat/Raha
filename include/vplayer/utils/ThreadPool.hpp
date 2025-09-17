#pragma once

#include "vplayer/utils/TaskQueue.hpp"

#include <atomic>
#include <functional>
#include <future>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace vplayer::utils {

class ThreadPool {
public:
    explicit ThreadPool(std::size_t thread_count = std::thread::hardware_concurrency());
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    template <typename Func, typename... Args>
    [[nodiscard]] auto enqueue(Func&& func, Args&&... args) {
        using Result = std::invoke_result_t<Func, Args...>;
        auto promise = std::make_shared<std::promise<Result>>();
        auto future = promise->get_future();

        queue_.push([promise, task = std::bind(std::forward<Func>(func), std::forward<Args>(args)...)]() mutable {
            try {
                if constexpr (std::is_void_v<Result>) {
                    std::invoke(task);
                    promise->set_value();
                } else {
                    promise->set_value(std::invoke(task));
                }
            } catch (...) {
                try {
                    promise->set_exception(std::current_exception());
                } catch (...) {
                }
            }
        });

        return future;
    }

    void wait_idle();

private:
    void worker_loop();

    std::vector<std::thread> workers_;
    TaskQueue queue_;
    std::atomic<bool> running_ {true};
};

} // namespace vplayer::utils
