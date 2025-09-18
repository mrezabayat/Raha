#include "raha/core/AudioRenderer.hpp"

#include "raha/utils/Logger.hpp"

extern "C" {
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
}

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace raha::core {

AudioRenderer::AudioRenderer() = default;
AudioRenderer::~AudioRenderer() { shutdown(); }

bool AudioRenderer::initialize(AVCodecContext* audio_ctx) {
    if (!audio_ctx) {
        return false;
    }
    if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            throw std::runtime_error(SDL_GetError());
        }
    }
    if (!configure_device(audio_ctx)) {
        return false;
    }
    resampler_ = make_resampler(audio_ctx);
    if (!resampler_) {
        return false;
    }
    SDL_PauseAudioDevice(device_, 0);
    return true;
}

void AudioRenderer::shutdown() {
    if (device_ != 0) {
        SDL_CloseAudioDevice(device_);
        device_ = 0;
    }
    resampler_.reset();
}

void AudioRenderer::queue_frame(const AVFrame* frame) {
    if (!frame || !resampler_ || device_ == 0) {
        return;
    }
    int out_samples = swr_get_out_samples(resampler_.get(), frame->nb_samples);
    std::vector<float> buffer(static_cast<std::size_t>(out_samples) * obtained_spec_.channels);
    uint8_t* out_planes[] = {reinterpret_cast<uint8_t*>(buffer.data())};
    const uint8_t** in_data = const_cast<const uint8_t**>(frame->extended_data);
    int converted = swr_convert(resampler_.get(), out_planes, out_samples, in_data, frame->nb_samples);
    if (converted < 0) {
        return;
    }
    std::size_t total_samples = static_cast<std::size_t>(converted) * obtained_spec_.channels;
    float applied_volume = muted_.load() ? 0.0F : volume_.load();
    if (applied_volume != 1.0F) {
        for (std::size_t i = 0; i < total_samples; ++i) {
            buffer[i] *= applied_volume;
        }
    }
    SDL_QueueAudio(device_, buffer.data(), total_samples * sizeof(float));
}

void AudioRenderer::clear() {
    if (device_ != 0) {
        SDL_ClearQueuedAudio(device_);
    }
}

void AudioRenderer::set_volume(float volume) {
    volume_.store(std::clamp(volume, 0.0F, 1.0F));
}

void AudioRenderer::set_muted(bool muted) {
    muted_.store(muted);
}

bool AudioRenderer::configure_device(const AVCodecContext* audio_ctx) {
    SDL_AudioSpec desired {};
    desired.freq = audio_ctx->sample_rate;
    desired.format = AUDIO_F32SYS;
    desired.channels = static_cast<Uint8>(audio_ctx->ch_layout.nb_channels);
    desired.samples = 4096;
    desired.callback = nullptr;

    device_ = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained_spec_, 0);
    if (device_ == 0) {
        utils::get_logger()->error("Failed to open audio device: {}", SDL_GetError());
        return false;
    }
    return true;
}

SwrContextPtr AudioRenderer::make_resampler(AVCodecContext* audio_ctx) {
    SwrContext* ctx = swr_alloc();
    if (!ctx) {
        return nullptr;
    }
    if (av_opt_set_chlayout(ctx, "in_chlayout", &audio_ctx->ch_layout, 0) < 0 ||
        av_opt_set_chlayout(ctx, "out_chlayout", &audio_ctx->ch_layout, 0) < 0 ||
        av_opt_set_int(ctx, "in_sample_rate", audio_ctx->sample_rate, 0) < 0 ||
        av_opt_set_int(ctx, "out_sample_rate", audio_ctx->sample_rate, 0) < 0 ||
        av_opt_set_sample_fmt(ctx, "in_sample_fmt", static_cast<AVSampleFormat>(audio_ctx->sample_fmt), 0) < 0 ||
        av_opt_set_sample_fmt(ctx, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0) < 0) {
        swr_free(&ctx);
        return nullptr;
    }
    if (swr_init(ctx) < 0) {
        swr_free(&ctx);
        return nullptr;
    }
    return SwrContextPtr(ctx);
}

} // namespace raha::core
