#include "raha/core/DecoderBridge.hpp"

#include "raha/utils/Logger.hpp"

extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

#include <stdexcept>

namespace raha::core {

namespace {
FramePtr make_frame() {
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        throw std::runtime_error("Failed to allocate AVFrame");
    }
    return FramePtr(frame);
}

void drain_frames(AVCodecContext* ctx, std::queue<FramePtr>& fifo) {
    auto logger = utils::get_logger();
    while (true) {
        FramePtr frame = make_frame();
        int ret = avcodec_receive_frame(ctx, frame.get());
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return;
        }
        if (ret < 0) {
            logger->error("Error receiving frame: {}", ret);
            return;
        }
        fifo.push(std::move(frame));
    }
}

} // namespace

DecoderBridge::DecoderBridge() = default;
DecoderBridge::~DecoderBridge() { shutdown(); }

bool DecoderBridge::prepare(MediaSource& source) {
    shutdown();
    source_ = &source;
    video_ctx_ = create_context(source, AVMEDIA_TYPE_VIDEO, source.video_stream_index());
    audio_ctx_ = create_context(source, AVMEDIA_TYPE_AUDIO, source.audio_stream_index());
    packet_.reset(av_packet_alloc());
    if (!packet_) {
        throw std::runtime_error("Failed to allocate AVPacket");
    }
    if (source.video_stream_index()) {
        video_stream_index_ = *source.video_stream_index();
    }
    if (source.audio_stream_index()) {
        audio_stream_index_ = *source.audio_stream_index();
    }
    eof_ = false;
    return true;
}

void DecoderBridge::shutdown() {
    video_ctx_.reset();
    audio_ctx_.reset();
    packet_.reset();
    source_ = nullptr;
    video_stream_index_ = -1;
    audio_stream_index_ = -1;
    eof_ = false;
    video_frames_ = {};
    audio_frames_ = {};
}

std::optional<FramePtr> DecoderBridge::next_video_frame() {
    if (!video_ctx_) {
        return std::nullopt;
    }
    while (video_frames_.empty()) {
        if (!source_) {
            return std::nullopt;
        }
        if (eof_) {
            drain_frames(video_ctx_.get(), video_frames_);
            if (video_frames_.empty()) {
                return std::nullopt;
            }
            break;
        }
        int ret = av_read_frame(source_->raw(), packet_.get());
        if (ret < 0) {
            eof_ = true;
            avcodec_send_packet(video_ctx_.get(), nullptr);
            if (audio_ctx_) {
                avcodec_send_packet(audio_ctx_.get(), nullptr);
            }
            av_packet_unref(packet_.get());
            continue;
        }
        if (packet_->stream_index == video_stream_index_) {
            avcodec_send_packet(video_ctx_.get(), packet_.get());
            drain_frames(video_ctx_.get(), video_frames_);
        } else if (audio_ctx_ && packet_->stream_index == audio_stream_index_) {
            avcodec_send_packet(audio_ctx_.get(), packet_.get());
            drain_frames(audio_ctx_.get(), audio_frames_);
        }
        av_packet_unref(packet_.get());
    }

    FramePtr frame = std::move(video_frames_.front());
    video_frames_.pop();
    return frame;
}

std::optional<FramePtr> DecoderBridge::next_audio_frame() {
    if (!audio_ctx_) {
        return std::nullopt;
    }
    while (audio_frames_.empty()) {
        if (!source_) {
            return std::nullopt;
        }
        if (eof_) {
            drain_frames(audio_ctx_.get(), audio_frames_);
            if (audio_frames_.empty()) {
                return std::nullopt;
            }
            break;
        }
        int ret = av_read_frame(source_->raw(), packet_.get());
        if (ret < 0) {
            eof_ = true;
            if (video_ctx_) {
                avcodec_send_packet(video_ctx_.get(), nullptr);
            }
            avcodec_send_packet(audio_ctx_.get(), nullptr);
            av_packet_unref(packet_.get());
            continue;
        }
        if (packet_->stream_index == audio_stream_index_) {
            avcodec_send_packet(audio_ctx_.get(), packet_.get());
            drain_frames(audio_ctx_.get(), audio_frames_);
        } else if (video_ctx_ && packet_->stream_index == video_stream_index_) {
            avcodec_send_packet(video_ctx_.get(), packet_.get());
            drain_frames(video_ctx_.get(), video_frames_);
        }
        av_packet_unref(packet_.get());
    }

    FramePtr frame = std::move(audio_frames_.front());
    audio_frames_.pop();
    return frame;
}

bool DecoderBridge::seek(double seconds) {
    if (!source_) {
        return false;
    }
    int64_t timestamp = static_cast<int64_t>(seconds * AV_TIME_BASE);
    if (av_seek_frame(source_->raw(), -1, timestamp, AVSEEK_FLAG_BACKWARD) < 0) {
        return false;
    }
    if (video_ctx_) {
        avcodec_flush_buffers(video_ctx_.get());
    }
    if (audio_ctx_) {
        avcodec_flush_buffers(audio_ctx_.get());
    }
    video_frames_ = {};
    audio_frames_ = {};
    eof_ = false;
    return true;
}

CodecContextPtr DecoderBridge::create_context(MediaSource& source, AVMediaType type, std::optional<int> index) {
    if (!index) {
        return nullptr;
    }
    AVStream* stream = source.raw()->streams[*index];
    const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) {
        throw std::runtime_error("Unsupported codec");
    }
    CodecContextPtr ctx(avcodec_alloc_context3(codec));
    if (!ctx) {
        throw std::runtime_error("Failed to allocate codec context");
    }
    if (avcodec_parameters_to_context(ctx.get(), stream->codecpar) < 0) {
        throw std::runtime_error("Failed to populate codec context");
    }
    ctx->pkt_timebase = stream->time_base;
    if (avcodec_open2(ctx.get(), codec, nullptr) < 0) {
        throw std::runtime_error("Failed to open codec context");
    }
    return ctx;
}

} // namespace raha::core
