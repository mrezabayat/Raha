#pragma once

extern "C" {
#include <libavutil/frame.h>
}

#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>

namespace vplayer::core {

struct FrameDeleter {
    void operator()(AVFrame* frame) const {
        av_frame_free(&frame);
    }
};

using FramePtr = std::unique_ptr<AVFrame, FrameDeleter>;

class FrameQueue {
public:
    explicit FrameQueue(std::size_t capacity = 10);

    void push(FramePtr frame);
    FramePtr pop();
    void clear();
    void stop();

private:
    std::size_t capacity_;
    std::queue<FramePtr> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_ {false};
};

} // namespace vplayer::core
