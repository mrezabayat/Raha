#pragma once

#include "raha/core/ApplicationConfig.hpp"
#include "raha/core/LibraryDatabase.hpp"
#include "raha/core/MediaPlayer.hpp"
#include "raha/core/PlaybackController.hpp"
#include "raha/core/PlaylistManager.hpp"
#include "raha/core/SeekController.hpp"

#include <SDL.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct SDL_Window;
struct SDL_Renderer;

namespace raha::frontend {

class App {
public:
    App();
    ~App();

    bool initialize(int width, int height, const std::string& title);
    void shutdown();
    void run();

    bool open_media(const std::string& uri);

private:
    void handle_event(const SDL_Event& event);
    void render_ui();
    void persist_state();

    SDL_Window* window_ {nullptr};
    raha::core::ApplicationConfig config_;
    raha::core::MediaPlayer player_;
    SDL_Renderer* renderer_ {nullptr};
    bool sdl_initialized_ {false};
    std::unique_ptr<raha::core::PlaybackController> playback_controller_;
    std::unique_ptr<raha::core::SeekController> seek_controller_;
    raha::core::PlaylistManager playlist_;
    raha::core::LibraryDatabase library_db_;

    bool running_ {false};
};

} // namespace raha::frontend
