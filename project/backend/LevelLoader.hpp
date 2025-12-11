#pragma once
#include "Level.hpp"
#include <string>

// Новий завантажувач рівнів у форматі worldview
Level loadLevelFromFile(const std::string& path);
