#include "vplayer/core/ScreenshotExporter.hpp"

#include "vplayer/utils/Logger.hpp"

extern "C" {
#include <libswscale/swscale.h>
}

#include <fstream>

namespace vplayer::core {

bool ScreenshotExporter::export_frame(const AVFrame* frame, const std::filesystem::path& path) const {
    if (!frame) {
        return false;
    }
    SwsContext* ctx = sws_getContext(
        frame->width,
        frame->height,
        static_cast<AVPixelFormat>(frame->format),
        frame->width,
        frame->height,
        AV_PIX_FMT_RGB24,
        SWS_BICUBIC,
        nullptr,
        nullptr,
        nullptr);
    if (!ctx) {
        return false;
    }

    AVFrame* rgb_frame = av_frame_alloc();
    if (!rgb_frame) {
        sws_freeContext(ctx);
        return false;
    }
    rgb_frame->format = AV_PIX_FMT_RGB24;
    rgb_frame->width = frame->width;
    rgb_frame->height = frame->height;
    if (av_frame_get_buffer(rgb_frame, 0) < 0) {
        av_frame_free(&rgb_frame);
        sws_freeContext(ctx);
        return false;
    }

    sws_scale(ctx, frame->data, frame->linesize, 0, frame->height, rgb_frame->data, rgb_frame->linesize);

    std::ofstream output(path, std::ios::binary);
    if (!output.is_open()) {
        av_frame_free(&rgb_frame);
        sws_freeContext(ctx);
        return false;
    }
    output << "P6\n" << frame->width << " " << frame->height << "\n255\n";
    output.write(reinterpret_cast<const char*>(rgb_frame->data[0]), frame->height * rgb_frame->linesize[0]);

    av_frame_free(&rgb_frame);
    sws_freeContext(ctx);
    return true;
}

} // namespace vplayer::core
