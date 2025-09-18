#pragma once

#include <memory>
#include <spdlog/logger.h>

namespace raha::utils {

std::shared_ptr<spdlog::logger> get_logger();
void init_logger();

} // namespace raha::utils
