#pragma once

#include "raha/core/ApplicationConfig.hpp"

extern "C" {
#include <libavutil/frame.h>
}

struct SwsContext;

#include <SDL.h>
#include <filesystem>
#include <optional>
#include <vector>

namespace raha::core {

class VideoRenderer {
public:
    VideoRenderer();
    ~VideoRenderer();

    VideoRenderer(const VideoRenderer&) = delete;
    VideoRenderer& operator=(const VideoRenderer&) = delete;

    bool initialize(SDL_Window* window, SDL_Renderer* renderer);
    void shutdown();

    void render_frame(const AVFrame* frame, const VideoAdjustments& adjustments);
    void resize(int width, int height);

    void request_screenshot(const std::filesystem::path& path);
    void present();

private:
    void apply_adjustments(const VideoAdjustments& adjustments);
    void ensure_texture(int width, int height, AVPixelFormat format);

    SDL_Window* window_ {nullptr};
    SDL_Renderer* renderer_ {nullptr};
    SDL_Texture* texture_ {nullptr};
    struct SwsContext* sws_ {nullptr};
    AVPixelFormat src_format_ {AV_PIX_FMT_NONE};
    int texture_width_ {0};
    int texture_height_ {0};
    std::vector<uint8_t> pixel_buffer_;
    bool has_frame_ {false};
    bool use_yuv_texture_ {false};
    std::optional<std::filesystem::path> pending_screenshot_;
};

} // namespace raha::core
