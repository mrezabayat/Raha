#pragma once

#include "vplayer/core/ApplicationConfig.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace vplayer::core {

class SubtitleManager {
public:
    SubtitleManager();
    ~SubtitleManager();

    bool initialize();
    void shutdown();

    bool load_from_file(const std::filesystem::path& path);
    void unload();

    void render(double time_seconds, const SubtitleSettings& settings);

private:
    std::optional<std::filesystem::path> current_track_;
};

} // namespace vplayer::core
