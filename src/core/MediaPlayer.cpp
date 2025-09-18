#include "raha/core/MediaPlayer.hpp"

#include "raha/utils/Logger.hpp"

extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/frame.h>
}

#include <filesystem>
#include <optional>
#include <stdexcept>

namespace raha::core {

namespace {
SDL_Window* validate_window(SDL_Window* window) {
    if (!window) {
        throw std::runtime_error("MediaPlayer requires a valid SDL_Window");
    }
    return window;
}

} // namespace

MediaPlayer::MediaPlayer() : workers_(2) {}
MediaPlayer::~MediaPlayer() { shutdown(); }

bool MediaPlayer::initialize(SDL_Window* window, SDL_Renderer* renderer) {
    validate_window(window);
    subtitle_manager_.initialize();
    video_renderer_.initialize(window, renderer);
    state_ = PlayerState::Idle;
    running_ = true;
    playback_clock_.stop();
    pending_video_frame_.reset();
    has_pending_video_ = false;
    return true;
}

void MediaPlayer::shutdown() {
    running_ = false;
    stop();
    audio_renderer_.shutdown();
    video_renderer_.shutdown();
    subtitle_manager_.shutdown();
    decoder_.shutdown();
    source_.close();
    playback_clock_.stop();
    pending_video_frame_.reset();
    has_pending_video_ = false;
    config_.last_position_seconds.reset();
    state_ = PlayerState::Idle;
}

bool MediaPlayer::open(const std::string& uri) {
    std::scoped_lock lock(playback_mutex_);
    utils::get_logger()->info("Opening media: {}", uri);
    if (!source_.open(uri)) {
        state_ = PlayerState::Error;
        return false;
    }
    if (!decoder_.prepare(source_)) {
        utils::get_logger()->error("Failed to prepare decoder");
        state_ = PlayerState::Error;
        return false;
    }
    if (!audio_renderer_.initialize(decoder_.audio_context())) {
        utils::get_logger()->warn("Audio renderer initialization failed");
    }
    state_ = PlayerState::Ready;
    config_.last_media_path = std::filesystem::path(uri);
    config_.last_position_seconds = 0.0;
    playback_clock_.stop();
    pending_video_frame_.reset();
    has_pending_video_ = false;
    return true;
}

void MediaPlayer::close() {
    stop();
    decoder_.shutdown();
    source_.close();
    playback_clock_.stop();
    pending_video_frame_.reset();
    has_pending_video_ = false;
    state_ = PlayerState::Idle;
}

void MediaPlayer::play() {
    if (state_ == PlayerState::Ready || state_ == PlayerState::Stopped) {
        double start_time = config_.last_position_seconds.value_or(0.0);
        playback_clock_.set_speed(config_.playback.playback_speed);
        playback_clock_.start(start_time);
        state_ = PlayerState::Playing;
    } else if (state_ == PlayerState::Paused) {
        playback_clock_.resume();
        state_ = PlayerState::Playing;
    }
}

void MediaPlayer::pause() {
    if (state_ == PlayerState::Playing) {
        state_ = PlayerState::Paused;
        playback_clock_.pause();
        config_.last_position_seconds = playback_clock_.current_time();
    }
}

void MediaPlayer::stop() {
    if (state_ == PlayerState::Playing || state_ == PlayerState::Paused) {
        state_ = PlayerState::Stopped;
    }
    audio_renderer_.clear();
    playback_clock_.stop();
    config_.last_position_seconds = 0.0;
    pending_video_frame_.reset();
    has_pending_video_ = false;
}

bool MediaPlayer::seek(double seconds) {
    if (!decoder_.seek(seconds)) {
        return false;
    }
    if (config_.subtitles.subtitle_delay != 0.0) {
        // For now we rely on libass internal timing.
    }
    audio_renderer_.clear();
    pending_video_frame_.reset();
    has_pending_video_ = false;
    config_.last_position_seconds = seconds;
    if (state_ == PlayerState::Playing) {
        playback_clock_.set_speed(config_.playback.playback_speed);
        playback_clock_.start(seconds);
    } else {
        playback_clock_.stop();
    }
    return true;
}

void MediaPlayer::set_playback_speed(double speed) {
    config_.playback.playback_speed = speed;
    playback_clock_.set_speed(speed);
}

double MediaPlayer::current_time() const {
    if (state_ == PlayerState::Playing) {
        return playback_clock_.current_time();
    }
    return config_.last_position_seconds.value_or(0.0);
}

void MediaPlayer::update() {
    if (state_ != PlayerState::Playing) {
        return;
    }
    const double clock_time = playback_clock_.current_time();
    config_.last_position_seconds = clock_time;

    auto video_index = source_.video_stream_index();
    AVStream* video_stream = nullptr;
    if (video_index) {
        video_stream = source_.raw()->streams[*video_index];
    }

    auto frame_time_seconds = [&](const AVFrame* frame) -> std::optional<double> {
        if (!video_stream) {
            return std::nullopt;
        }
        int64_t pts = frame->pts;
        if (pts == AV_NOPTS_VALUE) {
            pts = frame->best_effort_timestamp;
        }
        if (pts == AV_NOPTS_VALUE) {
            return std::nullopt;
        }
        return pts * av_q2d(video_stream->time_base);
    };

    auto try_render = [&](FramePtr& frame_holder, bool& has_frame, double& stored_pts) {
        if (!frame_holder) {
            has_frame = false;
            return false;
        }
        auto pts_opt = frame_time_seconds(frame_holder.get());
        double pts_value = pts_opt.value_or(clock_time);
        if (pts_value <= clock_time + video_sync_tolerance_) {
            video_renderer_.render_frame(frame_holder.get(), config_.video_adjustments);
            config_.last_position_seconds = pts_value;
            frame_holder.reset();
            has_frame = false;
            return true;
        }
        stored_pts = pts_value;
        has_frame = true;
        return false;
    };

    if (has_pending_video_) {
        if (!try_render(pending_video_frame_, has_pending_video_, pending_video_pts_)) {
            return;
        }
    }

    while (true) {
        auto video_frame_opt = decoder_.next_video_frame();
        if (!video_frame_opt) {
            break;
        }
        FramePtr frame = std::move(video_frame_opt.value());
        auto pts_opt = frame_time_seconds(frame.get());
        double pts_value = pts_opt.value_or(clock_time);
        if (pts_value > clock_time + video_sync_tolerance_) {
            pending_video_pts_ = pts_value;
            pending_video_frame_ = std::move(frame);
            has_pending_video_ = true;
            break;
        }
        video_renderer_.render_frame(frame.get(), config_.video_adjustments);
        config_.last_position_seconds = pts_value;
    }

    auto audio_frame_opt = decoder_.next_audio_frame();
    if (audio_frame_opt) {
        audio_renderer_.queue_frame(audio_frame_opt->get());
    }
}

void MediaPlayer::present() {
    video_renderer_.present();
}

void MediaPlayer::toggle_mute() {
    bool new_state = !audio_renderer_.muted();
    audio_renderer_.set_muted(new_state);
    config_.audio.muted = new_state;
}

void MediaPlayer::set_volume(float volume) {
    audio_renderer_.set_volume(volume);
    config_.audio.volume = volume;
}

void MediaPlayer::set_video_adjustments(const VideoAdjustments& adjustments) {
    config_.video_adjustments = adjustments;
}

void MediaPlayer::request_screenshot(const std::filesystem::path& path) {
    video_renderer_.request_screenshot(path);
}

} // namespace raha::core
