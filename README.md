# Raha (C++20 Video Player)

A modular, cross-platform media player named **Raha**, written in C++20. Raha integrates FFmpeg for demuxing/decoding, SDL2 for audio/video output, and SQLite for media library persistence. The project is scaffolded with CMake and Conan for dependency management.

## Feature Highlights

- Playback controls: play/pause/stop, relative seeking, basic frame stepping, adjustable playback speed (controller plumbing).
- Multi-format decoder backend wrapping FFmpeg with codec-agnostic stream discovery.
- SDL2-powered audio output with runtime volume/mute controls and automatic format conversion via libswresample.
- CPU-based YUV→RGBA conversion feeding an SDL2 texture renderer for on-screen video playback.
- Subtitle management scaffolding ready for libass integration.
- Screenshot exporter writing frames to portable pixmap (PPM) snapshots.
- Playlist and media library management with SQLite-backed metadata storage.
- Config persistence (JSON) capturing playback preferences, last session state, and media history.
- SDL-based application loop with drag-and-drop file support and basic keyboard shortcuts.

> **Note**: GPU video presentation through libplacebo and the polished UI/UX layer are intentionally left as future work; current video rendering is stubbed for developers to extend.

## Repository Layout

```
include/          Public headers grouped by domain (core, frontend, platform, utils)
src/              Engine and application implementation
resources/        Placeholder for shaders/skins/assets
cmake/            CMake helper modules (extend as needed)
tests/            GoogleTest-based unit tests
conanfile.py      Conan recipe describing third-party dependencies
CMakeLists.txt    Root CMake configuration
```

## Prerequisites

- C++20 capable compiler (Clang 16+, GCC 12+, MSVC 2022)
- CMake ≥ 3.26
- Conan ≥ 2.0
- Python ≥ 3.8 (for Conan)
- FFmpeg, SDL2, SQLite, spdlog, nlohmann_json (fetched automatically via Conan)

## Building with Conan + CMake

```bash
# (optional) create & activate a virtualenv for Conan
python -m venv .venv
source .venv/bin/activate

# Install third-party dependencies and toolchain files
conan install . --output-folder build --build=missing -s build_type=Release

# Configure using the generated preset
cmake --preset conan-release

# Compile the targets
cmake --build --preset conan-release

# (Optional) execute the unit tests
ctest --preset conan-release

# (Optional) run the install target (default prefix)
cmake --build --preset conan-release --target install
# or install to a specific prefix
cmake --install build --prefix dist
```

The Conan-generated preset selects Ninja/Unix Makefiles automatically for your platform. Override with `CMAKE_GENERATOR` or `cmake --preset conan-release -G "Xcode"` if you prefer another generator.

## Running

```bash
./build/raha <path-to-media-file>
```

Keyboard shortcuts:

- `Space` — Toggle play/pause
- `Esc` — Quit
- `←` / `→` — Seek ±5 seconds
- `A` / `S` — Step backward/forward one frame (approximation)
- `↑` / `↓` — Adjust master volume
- Drag & drop a file onto the window to open it

## Testing

A lightweight GoogleTest suite is provided for the timing clock component. Extend `tests/` with additional coverage (decoder bridges, playlist logic, database interactions) as functionality matures.

## Roadmap / Open Items

- Wire libplacebo into an actual swapchain (Vulkan/Direct3D/Metal) and present decoded video frames.
- Implement GPU-backed subtitle compositing and tone-mapping controls.
- Expand audio pipeline with synchronization, channel balance, and advanced filters.
- Harden playback clocking (AV sync), buffering, and network/dvd stream support.
- Flesh out playlist UI, metadata browsing, search/sort filtering, and remote streaming.
- Integrate a real UI toolkit (Dear ImGui, Qt, native menus) for accessibility and localization.
- Add comprehensive error handling, diagnostics, and automated test coverage.

## Contributing

Pull requests are welcome! Please run formatting/linting (if configured) and unit tests before submitting changes. Document any platform-specific steps or new dependencies in this README.
