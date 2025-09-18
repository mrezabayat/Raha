#include "raha/core/PlaylistManager.hpp"

#include <algorithm>
#include <random>

namespace raha::core {

void PlaylistManager::add(PlaylistEntry entry) {
    entries_.push_back(std::move(entry));
    if (!current_index_) {
        current_index_ = 0;
    }
}

void PlaylistManager::remove(std::size_t index) {
    if (index >= entries_.size()) {
        return;
    }
    entries_.erase(entries_.begin() + static_cast<std::ptrdiff_t>(index));
    if (entries_.empty()) {
        current_index_.reset();
    } else if (current_index_ && *current_index_ >= entries_.size()) {
        current_index_ = entries_.size() - 1;
    }
}

void PlaylistManager::clear() {
    entries_.clear();
    current_index_.reset();
}

std::optional<PlaylistEntry> PlaylistManager::current() const {
    if (!current_index_ || *current_index_ >= entries_.size()) {
        return std::nullopt;
    }
    return entries_[*current_index_];
}

void PlaylistManager::set_index(std::size_t index) {
    if (index < entries_.size()) {
        current_index_ = index;
    }
}

std::optional<PlaylistEntry> PlaylistManager::next(bool loop, bool shuffle) {
    if (entries_.empty()) {
        return std::nullopt;
    }
    if (shuffle) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<std::size_t> dist(0, entries_.size() - 1);
        current_index_ = dist(gen);
        return entries_[*current_index_];
    }
    if (!current_index_) {
        current_index_ = 0;
        return entries_[*current_index_];
    }
    std::size_t next_index = *current_index_ + 1;
    if (next_index >= entries_.size()) {
        if (loop) {
            next_index = 0;
        } else {
            current_index_.reset();
            return std::nullopt;
        }
    }
    current_index_ = next_index;
    return entries_[*current_index_];
}

std::optional<PlaylistEntry> PlaylistManager::previous(bool loop) {
    if (entries_.empty() || !current_index_) {
        return std::nullopt;
    }
    if (*current_index_ == 0) {
        if (loop) {
            current_index_ = entries_.size() - 1;
        } else {
            return entries_[0];
        }
    } else {
        --(*current_index_);
    }
    return entries_[*current_index_];
}

} // namespace raha::core
