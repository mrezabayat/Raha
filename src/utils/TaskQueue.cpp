#include "vplayer/utils/TaskQueue.hpp"

namespace vplayer::utils {

void TaskQueue::push(Task task) {
    {
        std::scoped_lock lock(mutex_);
        if (stop_) {
            return;
        }
        queue_.push(std::move(task));
    }
    cv_.notify_one();
}

bool TaskQueue::try_pop(Task& task) {
    std::scoped_lock lock(mutex_);
    if (queue_.empty()) {
        return false;
    }
    task = std::move(queue_.front());
    queue_.pop();
    return true;
}

TaskQueue::Task TaskQueue::wait_and_pop() {
    std::unique_lock lock(mutex_);
    cv_.wait(lock, [this] { return stop_ || !queue_.empty(); });
    if (stop_ && queue_.empty()) {
        return Task {};
    }
    Task task = std::move(queue_.front());
    queue_.pop();
    return task;
}

void TaskQueue::stop() {
    {
        std::scoped_lock lock(mutex_);
        stop_ = true;
    }
    cv_.notify_all();
}

bool TaskQueue::stopped() const {
    std::scoped_lock lock(mutex_);
    return stop_;
}

bool TaskQueue::empty() const {
    std::scoped_lock lock(mutex_);
    return queue_.empty();
}

} // namespace vplayer::utils
