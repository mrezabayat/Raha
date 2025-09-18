#pragma once

extern "C" {
#include <libavformat/avformat.h>
}

#include <optional>
#include <string>
#include <vector>

namespace raha::core {

struct StreamInfo {
    int index {-1};
    AVMediaType type {AVMEDIA_TYPE_UNKNOWN};
    std::string codec_name;
    std::string codec_long_name;
    int width {0};
    int height {0};
    double fps {0.0};
    double duration_seconds {0.0};
    int sample_rate {0};
    int channels {0};
};

class MediaSource {
public:
    MediaSource();
    ~MediaSource();

    MediaSource(const MediaSource&) = delete;
    MediaSource& operator=(const MediaSource&) = delete;

    bool open(const std::string& path);
    void close();

    [[nodiscard]] bool is_open() const { return format_ctx_ != nullptr; }
    [[nodiscard]] const std::string& uri() const { return uri_; }
    [[nodiscard]] const std::vector<StreamInfo>& streams() const { return streams_; }
    [[nodiscard]] std::optional<int> video_stream_index() const { return video_stream_index_; }
    [[nodiscard]] std::optional<int> audio_stream_index() const { return audio_stream_index_; }
    [[nodiscard]] std::optional<int> subtitle_stream_index() const { return subtitle_stream_index_; }
    [[nodiscard]] double duration_seconds() const;

    AVFormatContext* raw() { return format_ctx_; }
    const AVFormatContext* raw() const { return format_ctx_; }

private:
    void discover_streams();

    AVFormatContext* format_ctx_ {nullptr};
    std::string uri_;
    std::vector<StreamInfo> streams_;
    std::optional<int> video_stream_index_;
    std::optional<int> audio_stream_index_;
    std::optional<int> subtitle_stream_index_;
};

} // namespace raha::core
