#include "raha/utils/Logger.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <mutex>

namespace raha::utils {

namespace {
std::once_flag init_flag;
std::shared_ptr<spdlog::logger> global_logger;
} // namespace

std::shared_ptr<spdlog::logger> get_logger() {
    if (!global_logger) {
        init_logger();
    }
    return global_logger;
}

void init_logger() {
    std::call_once(init_flag, [] {
        global_logger = spdlog::stdout_color_mt("raha");
        global_logger->set_level(spdlog::level::info);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    });
}

} // namespace raha::utils
