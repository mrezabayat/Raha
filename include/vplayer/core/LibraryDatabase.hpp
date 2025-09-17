#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

struct sqlite3;

namespace vplayer::core {

struct MediaEntry {
    int64_t id {0};
    std::filesystem::path path;
    std::string title;
    double duration_seconds {0.0};
    std::string codec;
    std::string resolution;
};

class LibraryDatabase {
public:
    LibraryDatabase();
    ~LibraryDatabase();

    bool open(const std::filesystem::path& path);
    void close();

    void ensure_schema();
    void upsert_entry(const MediaEntry& entry);
    [[nodiscard]] std::vector<MediaEntry> search(const std::string& query) const;

private:
    sqlite3* db_ {nullptr};
};

} // namespace vplayer::core
