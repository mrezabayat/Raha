find_package(ffmpeg REQUIRED COMPONENTS avcodec avformat avutil swresample swscale)
find_package(SDL2 REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(raha_core
    core/ApplicationConfig.cpp
    core/Clock.cpp
    core/MediaPlayer.cpp
    core/MediaSource.cpp
    core/PlaybackController.cpp
    core/SeekController.cpp
    core/SubtitleManager.cpp
    core/VideoRenderer.cpp
    core/AudioRenderer.cpp
    core/DecoderBridge.cpp
    core/FrameQueue.cpp
    core/LibraryDatabase.cpp
    core/PlaylistManager.cpp
    core/ScreenshotExporter.cpp
    utils/ThreadPool.cpp
    utils/TaskQueue.cpp
    utils/Logger.cpp
)

target_include_directories(raha_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(raha_core
    PUBLIC
        ffmpeg::avcodec
        ffmpeg::avformat
        ffmpeg::avutil
        ffmpeg::swresample
        ffmpeg::swscale
        SDL2::SDL2
        SQLite::SQLite3
        spdlog::spdlog
        nlohmann_json::nlohmann_json
)

target_compile_features(raha_core PUBLIC cxx_std_20)

add_executable(raha
    main.cpp
    frontend/App.cpp
    frontend/ImGuiLayer.cpp
    platform/PlatformAbstraction.cpp
)

target_link_libraries(raha
    PRIVATE
        raha_core
)

install(TARGETS raha RUNTIME DESTINATION bin)
