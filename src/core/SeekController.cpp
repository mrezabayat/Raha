#include "raha/core/SeekController.hpp"

extern "C" {
#include <libavutil/avutil.h>
}

#include <algorithm>

namespace raha::core {

SeekController::SeekController(MediaPlayer& player) : player_(player) {}

bool SeekController::seek_relative(double delta_seconds) {
    double current = player_.current_time();
    return seek_absolute(std::clamp(current + delta_seconds, 0.0, player_.duration()));
}

bool SeekController::seek_absolute(double seconds) {
    return player_.seek(seconds);
}

bool SeekController::frame_step(int direction) {
    auto meta_stream = player_.source().video_stream_index();
    if (!meta_stream) {
        return false;
    }
    auto* stream = player_.source().raw()->streams[*meta_stream];
    double frame_duration = av_q2d(stream->r_frame_rate);
    if (frame_duration == 0.0) {
        frame_duration = 1.0 / 30.0;
    } else {
        frame_duration = 1.0 / frame_duration;
    }
    return seek_relative(direction * frame_duration);
}

} // namespace raha::core
