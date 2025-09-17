#include "vplayer/frontend/App.hpp"
#include "vplayer/platform/PlatformAbstraction.hpp"
#include "vplayer/utils/Logger.hpp"

#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    try {
        vplayer::utils::init_logger();
        vplayer::frontend::App app;
        if (!app.initialize(1280, 720, vplayer::platform::default_window_title())) {
            std::cerr << "Failed to initialize application" << std::endl;
            return 1;
        }
        if (argc > 1) {
            app.open_media(argv[1]);
        }
        app.run();
        app.shutdown();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
