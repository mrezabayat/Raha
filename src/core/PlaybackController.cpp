#include "raha/core/PlaybackController.hpp"

#include <algorithm>

namespace raha::core {

PlaybackController::PlaybackController(MediaPlayer& player) : player_(player) {}

void PlaybackController::toggle_play_pause() {
    if (player_.state() == PlayerState::Playing) {
        player_.pause();
    } else {
        player_.play();
    }
}

void PlaybackController::stop() {
    player_.stop();
}

void PlaybackController::faster() {
    auto speed = player_.config().playback.playback_speed;
    player_.set_playback_speed(std::min(speed + 0.25, 4.0));
}

void PlaybackController::slower() {
    auto speed = player_.config().playback.playback_speed;
    player_.set_playback_speed(std::max(speed - 0.25, 0.25));
}

void PlaybackController::normal_speed() {
    player_.set_playback_speed(1.0);
}

} // namespace raha::core
