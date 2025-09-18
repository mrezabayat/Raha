#include "raha/frontend/App.hpp"

#include "raha/platform/PlatformAbstraction.hpp"
#include "raha/utils/Logger.hpp"

#include <SDL.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <thread>

namespace raha::frontend {

namespace {
std::filesystem::path config_path() {
    auto dir = raha::platform::user_config_directory();
    std::filesystem::create_directories(dir);
    return dir / "config.json";
}

} // namespace

App::App() : playback_controller_(std::make_unique<raha::core::PlaybackController>(player_)),
             seek_controller_(std::make_unique<raha::core::SeekController>(player_)) {
    try {
        config_ = raha::core::ApplicationConfig::load(config_path());
    } catch (const std::exception& e) {
        raha::utils::get_logger()->warn("Config load failed: {}", e.what());
    }
    player_.set_config(config_);
}

App::~App() {
    shutdown();
}

bool App::initialize(int width, int height, const std::string& title) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0) {
        raha::utils::get_logger()->error("SDL_Init failed: {}", SDL_GetError());
        return false;
    }
    sdl_initialized_ = true;

    window_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window_) {
        raha::utils::get_logger()->error("Failed to create window: {}", SDL_GetError());
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) {
        raha::utils::get_logger()->warn("Falling back to software renderer: {}", SDL_GetError());
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    }

    if (!renderer_) {
        return false;
    }

    if (!player_.initialize(window_, renderer_)) {
        return false;
    }

    if (!library_db_.open(config_.database_path)) {
        raha::utils::get_logger()->warn("Failed to open media library database");
    }

    running_ = true;
    return true;
}

void App::shutdown() {
    bool was_running = running_;
    running_ = false;
    persist_state();
    player_.shutdown();
    library_db_.close();
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    if (sdl_initialized_) {
        SDL_Quit();
        sdl_initialized_ = false;
    }
}

void App::run() {
    using namespace std::chrono_literals;
    while (running_) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            handle_event(event);
        }
        player_.update();
        config_.last_position_seconds = player_.current_time();

        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderClear(renderer_);
        player_.present();
        render_ui();
        SDL_RenderPresent(renderer_);

        std::this_thread::sleep_for(10ms);
    }
}

bool App::open_media(const std::string& uri) {
    if (!player_.open(uri)) {
        return false;
    }
    player_.play();

    raha::core::MediaEntry entry;
    entry.path = uri;
    entry.title = std::filesystem::path(uri).stem().string();
    entry.duration_seconds = player_.duration();
    entry.codec = "unknown";
    if (player_.source().video_stream_index()) {
        auto* stream = player_.source().raw()->streams[*player_.source().video_stream_index()];
        entry.codec = avcodec_get_name(stream->codecpar->codec_id);
        entry.resolution = std::to_string(stream->codecpar->width) + "x" + std::to_string(stream->codecpar->height);
    }
    try {
        library_db_.upsert_entry(entry);
    } catch (const std::exception& e) {
        raha::utils::get_logger()->warn("Failed to persist media entry: {}", e.what());
    }
    playlist_.add({uri, entry.title, entry.duration_seconds});
    return true;
}

void App::handle_event(const SDL_Event& event) {
    switch (event.type) {
    case SDL_QUIT:
        running_ = false;
        break;
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_SPACE:
            playback_controller_->toggle_play_pause();
            break;
        case SDLK_ESCAPE:
            running_ = false;
            break;
        case SDLK_RIGHT:
            seek_controller_->seek_relative(5.0);
            break;
        case SDLK_LEFT:
            seek_controller_->seek_relative(-5.0);
            break;
        case SDLK_UP: {
            float volume = std::clamp(player_.config().audio.volume + 0.05F, 0.0F, 1.0F);
            player_.set_volume(volume);
            config_.audio.volume = volume;
            break;
        }
        case SDLK_DOWN: {
            float volume = std::clamp(player_.config().audio.volume - 0.05F, 0.0F, 1.0F);
            player_.set_volume(volume);
            config_.audio.volume = volume;
            break;
        }
        case SDLK_s:
            seek_controller_->frame_step(1);
            break;
        case SDLK_a:
            seek_controller_->frame_step(-1);
            break;
        default:
            break;
        }
        break;
    case SDL_DROPFILE:
        if (event.drop.file) {
            open_media(event.drop.file);
            SDL_free(event.drop.file);
        }
        break;
    default:
        break;
    }
}

void App::render_ui() {
    double current = player_.current_time();
    double total = player_.duration();
    int current_min = static_cast<int>(current / 60.0);
    int current_sec = static_cast<int>(std::fmod(current, 60.0));
    int total_min = static_cast<int>(total / 60.0);
    int total_sec = static_cast<int>(std::fmod(total, 60.0));

    std::string title = "Raha";
    switch (player_.state()) {
    case raha::core::PlayerState::Playing:
        title += " - Playing";
        break;
    case raha::core::PlayerState::Paused:
        title += " - Paused";
        break;
    case raha::core::PlayerState::Stopped:
        title += " - Stopped";
        break;
    default:
        break;
    }

    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), " [%02d:%02d / %02d:%02d]", current_min, current_sec, total_min, total_sec);
    title += buffer;
    SDL_SetWindowTitle(window_, title.c_str());
}

void App::persist_state() {
    try {
        config_ = player_.config();
        config_.save(config_path());
    } catch (const std::exception& e) {
        raha::utils::get_logger()->error("Failed to persist config: {}", e.what());
    }
}

} // namespace raha::frontend
