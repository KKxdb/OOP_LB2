#pragma once
#include "Level.hpp"
#include "Types.hpp"
#include <nlohmann/json.hpp>

class GameEngine {
public:
    GameEngine();

    void loadLevel(Level&& lvl);

    // роботи, які ставить гравець
    void spawnPlacedRobot(int x, int y, const std::string& type);

    // переносимо усі placedRobots → Level
    void commitPlacedRobots();

    void addRobot(std::unique_ptr<Robot> r);

    void addPlacedRobot(std::unique_ptr<Robot> r);

    // очистити роботи при виході
    void clearPlacedRobots();

    nlohmann::json getStateJson() const;

    void applyCommands(const std::vector<Command>& cmds);
    void update();

    const Level& getLevel() const { return level; }
    Level& getLevelMutable() { return level; }

private:
    Level level;
    Robot* findRobotById(int id);
};
