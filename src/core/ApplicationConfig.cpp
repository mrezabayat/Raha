#include "vplayer/core/ApplicationConfig.hpp"

#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

namespace vplayer::core {
namespace {
using json = nlohmann::json;

json to_json(const ApplicationConfig& config) {
    json j;
    j["playback"] = {
        {"playback_speed", config.playback.playback_speed},
        {"loop_single", config.playback.loop_single},
        {"shuffle", config.playback.shuffle}
    };
    j["video_adjustments"] = {
        {"brightness", config.video_adjustments.brightness},
        {"contrast", config.video_adjustments.contrast},
        {"saturation", config.video_adjustments.saturation},
        {"gamma", config.video_adjustments.gamma},
        {"aspect_ratio_mode", config.video_adjustments.aspect_ratio_mode}
    };
    j["audio"] = {
        {"volume", config.audio.volume},
        {"muted", config.audio.muted},
        {"balance", config.audio.balance},
        {"audio_delay", config.audio.audio_delay}
    };
    j["subtitles"] = {
        {"enabled", config.subtitles.enabled},
        {"subtitle_delay", config.subtitles.subtitle_delay},
        {"font_scale", config.subtitles.font_scale},
        {"color", config.subtitles.color},
        {"outline_color", config.subtitles.outline_color},
        {"position_offset", config.subtitles.position_offset}
    };
    j["network"] = {
        {"allow_streaming", config.network.allow_streaming},
        {"sandbox_streams", config.network.sandbox_streams}
    };
    if (config.last_media_path) {
        j["last_media_path"] = config.last_media_path->string();
    }
    if (config.last_position_seconds) {
        j["last_position_seconds"] = *config.last_position_seconds;
    }
    j["database_path"] = config.database_path.string();
    return j;
}

ApplicationConfig from_json(const json& j) {
    ApplicationConfig config;
    if (auto playback = j.find("playback"); playback != j.end()) {
        config.playback.playback_speed = playback->value("playback_speed", config.playback.playback_speed);
        config.playback.loop_single = playback->value("loop_single", config.playback.loop_single);
        config.playback.shuffle = playback->value("shuffle", config.playback.shuffle);
    }
    if (auto video = j.find("video_adjustments"); video != j.end()) {
        config.video_adjustments.brightness = video->value("brightness", config.video_adjustments.brightness);
        config.video_adjustments.contrast = video->value("contrast", config.video_adjustments.contrast);
        config.video_adjustments.saturation = video->value("saturation", config.video_adjustments.saturation);
        config.video_adjustments.gamma = video->value("gamma", config.video_adjustments.gamma);
        config.video_adjustments.aspect_ratio_mode = video->value("aspect_ratio_mode", config.video_adjustments.aspect_ratio_mode);
    }
    if (auto audio = j.find("audio"); audio != j.end()) {
        config.audio.volume = audio->value("volume", config.audio.volume);
        config.audio.muted = audio->value("muted", config.audio.muted);
        config.audio.balance = audio->value("balance", config.audio.balance);
        config.audio.audio_delay = audio->value("audio_delay", config.audio.audio_delay);
    }
    if (auto subtitles = j.find("subtitles"); subtitles != j.end()) {
        config.subtitles.enabled = subtitles->value("enabled", config.subtitles.enabled);
        config.subtitles.subtitle_delay = subtitles->value("subtitle_delay", config.subtitles.subtitle_delay);
        config.subtitles.font_scale = subtitles->value("font_scale", config.subtitles.font_scale);
        config.subtitles.color = subtitles->value("color", config.subtitles.color);
        config.subtitles.outline_color = subtitles->value("outline_color", config.subtitles.outline_color);
        config.subtitles.position_offset = subtitles->value("position_offset", config.subtitles.position_offset);
    }
    if (auto network = j.find("network"); network != j.end()) {
        config.network.allow_streaming = network->value("allow_streaming", config.network.allow_streaming);
        config.network.sandbox_streams = network->value("sandbox_streams", config.network.sandbox_streams);
    }
    if (auto path = j.find("last_media_path"); path != j.end()) {
        config.last_media_path = std::filesystem::path(path->get<std::string>());
    }
    if (auto position = j.find("last_position_seconds"); position != j.end()) {
        config.last_position_seconds = position->get<double>();
    }
    config.database_path = std::filesystem::path(j.value("database_path", config.database_path.string()));
    return config;
}

} // namespace

ApplicationConfig ApplicationConfig::load(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return {};
    }
    std::ifstream input(path);
    if (!input.is_open()) {
        throw std::runtime_error("Unable to open config file: " + path.string());
    }
    json j;
    input >> j;
    return from_json(j);
}

void ApplicationConfig::save(const std::filesystem::path& path) const {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path);
    if (!output.is_open()) {
        throw std::runtime_error("Unable to write config file: " + path.string());
    }
    output << to_json(*this).dump(2);
}

} // namespace vplayer::core
