#pragma once

#include "raha/core/FrameQueue.hpp"
#include "raha/core/MediaSource.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <memory>
#include <optional>
#include <queue>

namespace raha::core {

struct CodecContextDeleter {
    void operator()(AVCodecContext* ctx) const {
        avcodec_free_context(&ctx);
    }
};

struct PacketDeleter {
    void operator()(AVPacket* pkt) const {
        av_packet_free(&pkt);
    }
};

using CodecContextPtr = std::unique_ptr<AVCodecContext, CodecContextDeleter>;
using PacketPtr = std::unique_ptr<AVPacket, PacketDeleter>;

class DecoderBridge {
public:
    DecoderBridge();
    ~DecoderBridge();

    DecoderBridge(const DecoderBridge&) = delete;
    DecoderBridge& operator=(const DecoderBridge&) = delete;

    bool prepare(MediaSource& source);
    void shutdown();

    std::optional<FramePtr> next_video_frame();
    std::optional<FramePtr> next_audio_frame();

    bool seek(double seconds);

    [[nodiscard]] AVCodecContext* video_context() const { return video_ctx_.get(); }
    [[nodiscard]] AVCodecContext* audio_context() const { return audio_ctx_.get(); }

private:
    CodecContextPtr create_context(MediaSource& source, AVMediaType type, std::optional<int> index);

    MediaSource* source_ {nullptr};
    CodecContextPtr video_ctx_;
    CodecContextPtr audio_ctx_;
    PacketPtr packet_;
    std::queue<FramePtr> video_frames_;
    std::queue<FramePtr> audio_frames_;
    int video_stream_index_ {-1};
    int audio_stream_index_ {-1};
    bool eof_ {false};
};

} // namespace raha::core
