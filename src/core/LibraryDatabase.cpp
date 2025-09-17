#include "vplayer/core/LibraryDatabase.hpp"

#include "vplayer/utils/Logger.hpp"

#include <sqlite3.h>

#include <stdexcept>

namespace vplayer::core {

LibraryDatabase::LibraryDatabase() = default;
LibraryDatabase::~LibraryDatabase() { close(); }

bool LibraryDatabase::open(const std::filesystem::path& path) {
    if (sqlite3_open(path.string().c_str(), &db_) != SQLITE_OK) {
        utils::get_logger()->error("Failed to open database: {}", path.string());
        db_ = nullptr;
        return false;
    }
    ensure_schema();
    return true;
}

void LibraryDatabase::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

void LibraryDatabase::ensure_schema() {
    if (!db_) {
        return;
    }
    const char* ddl = R"SQL(
        CREATE TABLE IF NOT EXISTS media (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            path TEXT UNIQUE NOT NULL,
            title TEXT,
            duration_seconds REAL,
            codec TEXT,
            resolution TEXT
        );
    )SQL";
    char* err = nullptr;
    if (sqlite3_exec(db_, ddl, nullptr, nullptr, &err) != SQLITE_OK) {
        std::string message = err ? err : "unknown";
        sqlite3_free(err);
        throw std::runtime_error("Failed to create schema: " + message);
    }
}

void LibraryDatabase::upsert_entry(const MediaEntry& entry) {
    if (!db_) {
        throw std::runtime_error("Database not opened");
    }
    const char* sql = R"SQL(
        INSERT INTO media (path, title, duration_seconds, codec, resolution)
        VALUES (?1, ?2, ?3, ?4, ?5)
        ON CONFLICT(path) DO UPDATE SET
            title = excluded.title,
            duration_seconds = excluded.duration_seconds,
            codec = excluded.codec,
            resolution = excluded.resolution;
    )SQL";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement");
    }
    sqlite3_bind_text(stmt, 1, entry.path.string().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, entry.title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, entry.duration_seconds);
    sqlite3_bind_text(stmt, 4, entry.codec.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, entry.resolution.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Failed to execute insert");
    }
    sqlite3_finalize(stmt);
}

std::vector<MediaEntry> LibraryDatabase::search(const std::string& query) const {
    std::vector<MediaEntry> results;
    if (!db_) {
        return results;
    }
    const char* sql = R"SQL(
        SELECT id, path, title, duration_seconds, codec, resolution
        FROM media
        WHERE title LIKE ?1 OR path LIKE ?1
        ORDER BY title ASC;
    )SQL";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare query");
    }
    std::string pattern = "%" + query + "%";
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        MediaEntry entry;
        entry.id = sqlite3_column_int64(stmt, 0);
        entry.path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        entry.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        entry.duration_seconds = sqlite3_column_double(stmt, 3);
        entry.codec = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        entry.resolution = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        results.push_back(std::move(entry));
    }

    sqlite3_finalize(stmt);
    return results;
}

} // namespace vplayer::core
