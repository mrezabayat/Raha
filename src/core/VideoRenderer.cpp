#include "vplayer/core/VideoRenderer.hpp"

#include "vplayer/core/ScreenshotExporter.hpp"
#include "vplayer/utils/Logger.hpp"

extern "C" {
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <cmath>
#include <stdexcept>
#include <string>

namespace vplayer::core {

VideoRenderer::VideoRenderer() = default;
VideoRenderer::~VideoRenderer() { shutdown(); }

bool VideoRenderer::initialize(SDL_Window* window, SDL_Renderer* renderer) {
    window_ = window;
    renderer_ = renderer ? renderer : SDL_GetRenderer(window);
    if (!window_ || !renderer_) {
        throw std::runtime_error("VideoRenderer requires valid SDL window and renderer");
    }
    utils::get_logger()->info("Video renderer initialised (SDL texture pipeline)");
    return true;
}

void VideoRenderer::shutdown() {
    if (texture_) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
    if (sws_) {
        sws_freeContext(sws_);
        sws_ = nullptr;
    }
    window_ = nullptr;
    renderer_ = nullptr;
    texture_width_ = 0;
    texture_height_ = 0;
    pixel_buffer_.clear();
    has_frame_ = false;
    pending_screenshot_.reset();
}

void VideoRenderer::render_frame(const AVFrame* frame, const VideoAdjustments& adjustments) {
    if (!frame || !renderer_) {
        return;
    }
    apply_adjustments(adjustments);

    ensure_texture(frame->width, frame->height, static_cast<AVPixelFormat>(frame->format));

    if (use_yuv_texture_) {
        SDL_UpdateYUVTexture(texture_, nullptr,
            frame->data[0], frame->linesize[0],
            frame->data[1], frame->linesize[1],
            frame->data[2], frame->linesize[2]);
    } else {
        uint8_t* dest_slices[4] {pixel_buffer_.data(), nullptr, nullptr, nullptr};
        int dest_linesize[4] {texture_width_ * 4, 0, 0, 0};

        const uint8_t* src_data[4];
        int src_linesize[4];
        for (int i = 0; i < 4; ++i) {
            src_data[i] = frame->data[i];
            src_linesize[i] = frame->linesize[i];
        }

        sws_scale(sws_, src_data, src_linesize, 0, frame->height, dest_slices, dest_linesize);
        SDL_UpdateTexture(texture_, nullptr, pixel_buffer_.data(), texture_width_ * 4);
    }
    has_frame_ = true;

    if (pending_screenshot_) {
        ScreenshotExporter exporter;
        if (exporter.export_frame(frame, *pending_screenshot_)) {
            utils::get_logger()->info("Screenshot written to {}", pending_screenshot_->string());
        } else {
            utils::get_logger()->warn("Failed to write screenshot to {}", pending_screenshot_->string());
        }
        pending_screenshot_.reset();
    }
}

void VideoRenderer::present() {
    if (!renderer_ || !texture_ || !has_frame_) {
        return;
    }
    SDL_Rect dest {0, 0, texture_width_, texture_height_};
    int window_w = 0;
    int window_h = 0;
    SDL_GetRendererOutputSize(renderer_, &window_w, &window_h);
    if (window_w > 0 && window_h > 0) {
        double window_ratio = static_cast<double>(window_w) / static_cast<double>(window_h);
        double video_ratio = static_cast<double>(texture_width_) / static_cast<double>(texture_height_);
        if (std::abs(window_ratio - video_ratio) > 0.01) {
            if (window_ratio > video_ratio) {
                dest.h = window_h;
                dest.w = static_cast<int>(window_h * video_ratio);
                dest.x = (window_w - dest.w) / 2;
                dest.y = 0;
            } else {
                dest.w = window_w;
                dest.h = static_cast<int>(window_w / video_ratio);
                dest.x = 0;
                dest.y = (window_h - dest.h) / 2;
            }
        } else {
            dest = SDL_Rect {0, 0, window_w, window_h};
        }
    }
    SDL_RenderCopy(renderer_, texture_, nullptr, &dest);
}

void VideoRenderer::resize(int width, int height) {
    utils::get_logger()->info("Resize requested: {}x{}", width, height);
    (void)width;
    (void)height;
}

void VideoRenderer::request_screenshot(const std::filesystem::path& path) {
    pending_screenshot_ = path;
}

void VideoRenderer::apply_adjustments(const VideoAdjustments& adjustments) {
    (void)adjustments;
    // TODO: Integrate shader-based tone mapping and post-processing.
}

void VideoRenderer::ensure_texture(int width, int height, AVPixelFormat format) {
    if (!renderer_) {
        return;
    }
    bool format_changed = (format != src_format_);
    if (width != texture_width_ || height != texture_height_ || format_changed) {
        texture_width_ = width;
        texture_height_ = height;
        src_format_ = format;
        if (texture_) {
            SDL_DestroyTexture(texture_);
            texture_ = nullptr;
        }

        bool can_use_yuv = (format == AV_PIX_FMT_YUV420P || format == AV_PIX_FMT_YUVJ420P);
        use_yuv_texture_ = can_use_yuv;
        if (use_yuv_texture_) {
            pixel_buffer_.clear();
            if (sws_) {
                sws_freeContext(sws_);
                sws_ = nullptr;
            }
            texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
        } else {
            pixel_buffer_.resize(static_cast<std::size_t>(width) * height * 4);
            texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING, width, height);
        }

        if (!texture_) {
            throw std::runtime_error(std::string("Failed to create SDL texture: ") + SDL_GetError());
        }
    }

    if (!use_yuv_texture_) {
        sws_ = sws_getCachedContext(sws_, width, height, format, width, height, AV_PIX_FMT_BGRA, SWS_BILINEAR, nullptr, nullptr, nullptr);
        if (!sws_) {
            throw std::runtime_error("Failed to acquire swscale context");
        }
    }
}

} // namespace vplayer::core
