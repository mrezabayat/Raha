#pragma once

namespace raha::frontend {

class ImGuiLayer {
public:
    bool initialize(void*) { return true; }
    void shutdown() {}
    void new_frame() {}
    void render() {}
};

} // namespace raha::frontend
