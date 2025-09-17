#pragma once

#include <memory>
#include <spdlog/logger.h>

namespace vplayer::utils {

std::shared_ptr<spdlog::logger> get_logger();
void init_logger();

} // namespace vplayer::utils
