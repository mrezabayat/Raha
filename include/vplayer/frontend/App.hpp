#pragma once

#include "vplayer/core/ApplicationConfig.hpp"
#include "vplayer/core/LibraryDatabase.hpp"
#include "vplayer/core/MediaPlayer.hpp"
#include "vplayer/core/PlaybackController.hpp"
#include "vplayer/core/PlaylistManager.hpp"
#include "vplayer/core/SeekController.hpp"

#include <SDL.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct SDL_Window;
struct SDL_Renderer;

namespace vplayer::frontend {

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
    vplayer::core::ApplicationConfig config_;
    vplayer::core::MediaPlayer player_;
    SDL_Renderer* renderer_ {nullptr};
    bool sdl_initialized_ {false};
    std::unique_ptr<vplayer::core::PlaybackController> playback_controller_;
    std::unique_ptr<vplayer::core::SeekController> seek_controller_;
    vplayer::core::PlaylistManager playlist_;
    vplayer::core::LibraryDatabase library_db_;

    bool running_ {false};
};

} // namespace vplayer::frontend
