#pragma once

#include "raha/core/MediaPlayer.hpp"

namespace raha::core {

class SeekController {
public:
    explicit SeekController(MediaPlayer& player);

    bool seek_relative(double delta_seconds);
    bool seek_absolute(double seconds);
    bool frame_step(int direction);

private:
    MediaPlayer& player_;
};

} // namespace raha::core
