#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>

namespace vplayer::utils {

class TaskQueue {
public:
    using Task = std::function<void()>;

    void push(Task task);
    bool try_pop(Task& task);
    Task wait_and_pop();
    void stop();
    bool stopped() const;
    bool empty() const;

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<Task> queue_;
    bool stop_ {false};
};

} // namespace vplayer::utils
