#include "vplayer/platform/PlatformAbstraction.hpp"

#include <cstdlib>

namespace vplayer::platform {

std::filesystem::path user_config_directory() {
#if defined(_WIN32)
    const char* appdata = std::getenv("APPDATA");
    if (appdata) {
        return std::filesystem::path(appdata) / "VideoPlayer";
    }
    return std::filesystem::current_path();
#elif defined(__APPLE__)
    const char* home = std::getenv("HOME");
    if (home) {
        return std::filesystem::path(home) / "Library/Application Support/VideoPlayer";
    }
    return std::filesystem::current_path();
#else
    const char* home = std::getenv("HOME");
    if (home) {
        return std::filesystem::path(home) / ".config/video-player";
    }
    return std::filesystem::current_path();
#endif
}

std::string default_window_title() {
    return "Video Player";
}

} // namespace vplayer::platform
