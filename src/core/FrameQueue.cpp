#include "vplayer/core/FrameQueue.hpp"

#include <stdexcept>

namespace vplayer::core {

FrameQueue::FrameQueue(std::size_t capacity) : capacity_(capacity) {
    if (capacity_ == 0) {
        throw std::invalid_argument("FrameQueue capacity must be greater than zero");
    }
}

void FrameQueue::push(FramePtr frame) {
    std::unique_lock lock(mutex_);
    cv_.wait(lock, [this] { return stop_ || queue_.size() < capacity_; });
    if (stop_) {
        return;
    }
    queue_.push(std::move(frame));
    lock.unlock();
    cv_.notify_all();
}

FramePtr FrameQueue::pop() {
    std::unique_lock lock(mutex_);
    cv_.wait(lock, [this] { return stop_ || !queue_.empty(); });
    if (stop_ && queue_.empty()) {
        return {};
    }
    auto frame = std::move(queue_.front());
    queue_.pop();
    lock.unlock();
    cv_.notify_all();
    return frame;
}

void FrameQueue::clear() {
    std::scoped_lock lock(mutex_);
    while (!queue_.empty()) {
        queue_.pop();
    }
}

void FrameQueue::stop() {
    {
        std::scoped_lock lock(mutex_);
        stop_ = true;
        clear();
    }
    cv_.notify_all();
}

} // namespace vplayer::core
