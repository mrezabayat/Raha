#pragma once

#include <filesystem>

extern "C" {
#include <libavutil/frame.h>
}

namespace raha::core {

class ScreenshotExporter {
public:
    bool export_frame(const AVFrame* frame, const std::filesystem::path& path) const;
};

} // namespace raha::core
