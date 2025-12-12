#pragma once
#include "Level.hpp"
#include "Types.hpp"
#include <nlohmann/json.hpp>
#include <vector>


class GameEngine {
public:
    
    GameEngine();

    void loadLevel(Level&& lvl);

    nlohmann::json getStateJson() const;

    void applyCommands(const std::vector<Command>& cmds);

    void update();

    const Level& getLevel() const { return level_; }
    Level& getLevelMutable() { return level_; }
    void addRobot(std::unique_ptr<Robot> r) { level.addRobot(std::move(r)); }

private:
    Level level;
    Level level_;                      // правильно
    std::vector<RobotState> robots_;   // потрібно для getStateJson()
    int moves_ = 0;                    // потрібно для getStateJson()

    Robot* findRobotById(int id);
};
