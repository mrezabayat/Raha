#include "raha/core/MediaSource.hpp"

#include "raha/utils/Logger.hpp"

extern "C" {
#include <libavutil/avutil.h>
}

#include <stdexcept>

namespace raha::core {

MediaSource::MediaSource() = default;

MediaSource::~MediaSource() {
    close();
}

bool MediaSource::open(const std::string& path) {
    close();

    auto logger = utils::get_logger();
    logger->info("Opening media source: {}", path);

    if (avformat_open_input(&format_ctx_, path.c_str(), nullptr, nullptr) < 0) {
        logger->error("Failed to open media source: {}", path);
        format_ctx_ = nullptr;
        return false;
    }

    if (avformat_find_stream_info(format_ctx_, nullptr) < 0) {
        logger->error("Failed to read stream info: {}", path);
        close();
        return false;
    }

    uri_ = path;
    discover_streams();
    return true;
}

void MediaSource::close() {
    if (format_ctx_) {
        avformat_close_input(&format_ctx_);
        format_ctx_ = nullptr;
    }
    streams_.clear();
    uri_.clear();
    video_stream_index_.reset();
    audio_stream_index_.reset();
    subtitle_stream_index_.reset();
}

double MediaSource::duration_seconds() const {
    if (!format_ctx_ || format_ctx_->duration == AV_NOPTS_VALUE) {
        return 0.0;
    }
    return static_cast<double>(format_ctx_->duration) / AV_TIME_BASE;
}

void MediaSource::discover_streams() {
    streams_.clear();
    video_stream_index_.reset();
    audio_stream_index_.reset();
    subtitle_stream_index_.reset();

    auto logger = utils::get_logger();
    for (unsigned int i = 0; i < format_ctx_->nb_streams; ++i) {
        auto* stream = format_ctx_->streams[i];
        auto* codec_params = stream->codecpar;
        StreamInfo info;
        info.index = static_cast<int>(i);
        info.type = static_cast<AVMediaType>(codec_params->codec_type);
        const AVCodec* codec = avcodec_find_decoder(codec_params->codec_id);
        if (codec) {
            info.codec_name = codec->name ? codec->name : "";
            info.codec_long_name = codec->long_name ? codec->long_name : "";
        }
        if (info.type == AVMEDIA_TYPE_VIDEO) {
            info.width = codec_params->width;
            info.height = codec_params->height;
            if (stream->avg_frame_rate.num != 0) {
                info.fps = av_q2d(stream->avg_frame_rate);
            }
            video_stream_index_ = info.index;
        } else if (info.type == AVMEDIA_TYPE_AUDIO) {
            info.sample_rate = codec_params->sample_rate;
            info.channels = codec_params->ch_layout.nb_channels;
            audio_stream_index_ = info.index;
        } else if (info.type == AVMEDIA_TYPE_SUBTITLE) {
            subtitle_stream_index_ = info.index;
        }
        if (stream->duration != AV_NOPTS_VALUE) {
            info.duration_seconds = stream->duration * av_q2d(stream->time_base);
        } else {
            info.duration_seconds = duration_seconds();
        }
        streams_.push_back(info);
        logger->debug("Discovered stream {} type {} codec {}", info.index, av_get_media_type_string(info.type), info.codec_name);
    }
}

} // namespace raha::core
