#pragma once

#include <filesystem>

extern "C" {
#include <libavutil/frame.h>
}

namespace vplayer::core {

class ScreenshotExporter {
public:
    bool export_frame(const AVFrame* frame, const std::filesystem::path& path) const;
};

} // namespace vplayer::core
