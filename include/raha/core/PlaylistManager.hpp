#pragma once

#include <optional>
#include <string>
#include <vector>

namespace raha::core {

struct PlaylistEntry {
    std::string uri;
    std::string title;
    double duration_seconds {0.0};
};

class PlaylistManager {
public:
    void add(PlaylistEntry entry);
    void remove(std::size_t index);
    void clear();

    [[nodiscard]] std::optional<PlaylistEntry> current() const;
    [[nodiscard]] bool empty() const { return entries_.empty(); }
    [[nodiscard]] std::size_t size() const { return entries_.size(); }

    void set_index(std::size_t index);
    std::optional<PlaylistEntry> next(bool loop, bool shuffle);
    std::optional<PlaylistEntry> previous(bool loop);

private:
    std::vector<PlaylistEntry> entries_;
    std::optional<std::size_t> current_index_;
};

} // namespace raha::core
