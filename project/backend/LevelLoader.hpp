#pragma once
#include "Level.hpp"
#include <string>

// Завантаження рівня з JSON-файлу
Level loadLevelFromFile(const std::string& path);