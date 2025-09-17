#pragma once

#include <filesystem>
#include <string>

namespace vplayer::platform {

std::filesystem::path user_config_directory();
std::string default_window_title();

} // namespace vplayer::platform
