#pragma once

#include "vplayer/core/DecoderBridge.hpp"

extern "C" {
#include <libswresample/swresample.h>
}

#include <SDL.h>
#include <atomic>
#include <memory>

namespace vplayer::core {

struct SwrContextDeleter {
    void operator()(SwrContext* ctx) const {
        swr_free(&ctx);
    }
};

using SwrContextPtr = std::unique_ptr<SwrContext, SwrContextDeleter>;

class AudioRenderer {
public:
    AudioRenderer();
    ~AudioRenderer();

    AudioRenderer(const AudioRenderer&) = delete;
    AudioRenderer& operator=(const AudioRenderer&) = delete;

    bool initialize(AVCodecContext* audio_ctx);
    void shutdown();

    void queue_frame(const AVFrame* frame);
    void clear();

    void set_volume(float volume);
    void set_muted(bool muted);
    [[nodiscard]] float volume() const { return volume_; }
    [[nodiscard]] bool muted() const { return muted_; }

private:
    bool configure_device(const AVCodecContext* audio_ctx);
    SwrContextPtr make_resampler(AVCodecContext* audio_ctx);

    SDL_AudioDeviceID device_ {0};
    SDL_AudioSpec obtained_spec_ {};
    SwrContextPtr resampler_;
    std::atomic<float> volume_ {1.0F};
    std::atomic<bool> muted_ {false};
};

} // namespace vplayer::core
