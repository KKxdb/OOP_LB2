#pragma once
#include <string>
#include <optional>
#include "Level.hpp"

class LevelLoader {
public:
    static std::optional<Level> loadFromJson(const std::string& path);
};