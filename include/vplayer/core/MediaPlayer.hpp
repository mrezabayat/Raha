#pragma once

#include "vplayer/core/ApplicationConfig.hpp"
#include "vplayer/core/AudioRenderer.hpp"
#include "vplayer/core/Clock.hpp"
#include "vplayer/core/DecoderBridge.hpp"
#include "vplayer/core/FrameQueue.hpp"
#include "vplayer/core/MediaSource.hpp"
#include "vplayer/core/SubtitleManager.hpp"
#include "vplayer/core/VideoRenderer.hpp"
#include "vplayer/utils/ThreadPool.hpp"

#include <SDL.h>
#include <atomic>
#include <filesystem>
#include <mutex>
#include <optional>
#include <string>

namespace vplayer::core {

enum class PlayerState {
    Idle,
    Ready,
    Playing,
    Paused,
    Stopped,
    Error
};

class MediaPlayer {
public:
    MediaPlayer();
    ~MediaPlayer();

    bool initialize(SDL_Window* window, SDL_Renderer* renderer);
    void shutdown();

    bool open(const std::string& uri);
    void close();

    void play();
    void pause();
    void stop();

    bool seek(double seconds);
    void set_playback_speed(double speed);

    void update();
    void present();

    [[nodiscard]] PlayerState state() const { return state_; }
    [[nodiscard]] const MediaSource& source() const { return source_; }
    [[nodiscard]] double current_time() const;
    [[nodiscard]] double duration() const { return source_.duration_seconds(); }

    void set_config(ApplicationConfig config) { config_ = std::move(config); }
    [[nodiscard]] const ApplicationConfig& config() const { return config_; }

    void toggle_mute();
    void set_volume(float volume);
    void set_video_adjustments(const VideoAdjustments& adjustments);

    void request_screenshot(const std::filesystem::path& path);

private:
    ApplicationConfig config_;
    MediaSource source_;
    DecoderBridge decoder_;
    SubtitleManager subtitle_manager_;
    VideoRenderer video_renderer_;
    AudioRenderer audio_renderer_;
    utils::ThreadPool workers_;

    std::atomic<PlayerState> state_ {PlayerState::Idle};
    std::atomic<bool> running_ {true};
    std::mutex playback_mutex_;

    Clock playback_clock_;
    FramePtr pending_video_frame_;
    bool has_pending_video_ {false};
    double pending_video_pts_ {0.0};
    const double video_sync_tolerance_ {0.02};
};

} // namespace vplayer::core
