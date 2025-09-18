#pragma once

#include "raha/core/MediaPlayer.hpp"

namespace raha::core {

class PlaybackController {
public:
    explicit PlaybackController(MediaPlayer& player);

    void toggle_play_pause();
    void stop();
    void faster();
    void slower();
    void normal_speed();

private:
    MediaPlayer& player_;
};

} // namespace raha::core
