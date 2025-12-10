#pragma once
#include <string>
#include "Level.hpp"
#include "Types.hpp"

// Простий обробник JSON-команд (рядок → рядок)
class GameEngine {
public:
    GameEngine();

    // Приймає JSON-рядок, повертає JSON-рядок зі станом
    std::string processCommand(const std::string& data);

    // нові методи
    void loadLevel( Level&& lvl);                 // завантажити рівень
    nlohmann::json getStateJson() const;              // отримати стан рівня
    void applyCommands(const std::vector<Command>&);  // виконати список команд



private:
    Level level;

    // знайти робота за його id
    Robot* findRobotById(int id);
};