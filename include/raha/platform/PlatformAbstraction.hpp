#pragma once

#include <filesystem>
#include <string>

namespace raha::platform {

std::filesystem::path user_config_directory();
std::string default_window_title();

} // namespace raha::platform
