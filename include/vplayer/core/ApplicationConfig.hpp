#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace vplayer::core {

struct PlaybackSettings {
    double playback_speed {1.0};
    bool loop_single {false};
    bool shuffle {false};
};

struct VideoAdjustments {
    float brightness {0.0F};
    float contrast {0.0F};
    float saturation {0.0F};
    float gamma {0.0F};
    int aspect_ratio_mode {0};
};

struct AudioSettings {
    float volume {1.0F};
    bool muted {false};
    float balance {0.0F};
    double audio_delay {0.0};
};

struct SubtitleSettings {
    bool enabled {true};
    double subtitle_delay {0.0};
    float font_scale {1.0F};
    std::string color {"#FFFFFFFF"};
    std::string outline_color {"#FF000000"};
    float position_offset {0.0F};
};

struct NetworkSettings {
    bool allow_streaming {true};
    bool sandbox_streams {true};
};

struct ApplicationConfig {
    PlaybackSettings playback;
    VideoAdjustments video_adjustments;
    AudioSettings audio;
    SubtitleSettings subtitles;
    NetworkSettings network;

    std::optional<std::filesystem::path> last_media_path;
    std::optional<double> last_position_seconds;
    std::filesystem::path database_path {"video-player.db"};

    static ApplicationConfig load(const std::filesystem::path& path);
    void save(const std::filesystem::path& path) const;
};

} // namespace vplayer::core
