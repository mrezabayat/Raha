#include "raha/core/SubtitleManager.hpp"

#include "raha/utils/Logger.hpp"

namespace raha::core {

SubtitleManager::SubtitleManager() = default;
SubtitleManager::~SubtitleManager() = default;

bool SubtitleManager::initialize() {
    // libass integration is not yet available in this build.
    return true;
}

void SubtitleManager::shutdown() {
    current_track_.reset();
}

bool SubtitleManager::load_from_file(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        raha::utils::get_logger()->warn("Subtitle file not found: {}", path.string());
        return false;
    }
    current_track_ = path;
    raha::utils::get_logger()->info("Subtitle track queued: {}", path.string());
    return true;
}

void SubtitleManager::unload() {
    current_track_.reset();
}

void SubtitleManager::render(double time_seconds, const SubtitleSettings& settings) {
    (void)time_seconds;
    (void)settings;
    if (!current_track_) {
        return;
    }
    // Future work: integrate libass to render subtitles to video frames.
}

} // namespace raha::core
