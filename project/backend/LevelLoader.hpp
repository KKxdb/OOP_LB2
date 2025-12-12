#pragma once
#include <string>
#include <optional>
#include "Level.hpp"

class LevelLoader {
public:
    // Load level from JSON file
    static std::optional<Level> loadFromJson(const std::string& path);
};