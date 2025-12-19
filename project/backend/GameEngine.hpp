#pragma once
#include "Level.hpp"
#include "Types.hpp"
#include <nlohmann/json.hpp>

class GameEngine {
public:
    GameEngine();

    bool isWin() const;
    bool isLose() const;

    bool locked_ = false;
    //bool isLocked() const { return locked_; }
    void lock() { locked_ = true; }

    void stepAuto();
    void loadLevel(Level&& lvl);

    void spawnPlacedRobot(int x, int y, const std::string& type);
    void commitPlacedRobots();

    void addRobot(std::unique_ptr<Robot> r);
    void addPlacedRobot(std::unique_ptr<Robot> r);

    void clearPlacedRobots();

    nlohmann::json getStateJson() const;

    void applyCommands(const std::vector<Command>& cmds);
    void update();

    const Level& getLevel() const { return level; }
    Level& getLevelMutable() { return level; }

private:
    Level level;

    Robot* findRobotById(int id);
    std::vector<Command> collectControllerCommands();  

    static bool bfs_reachable(const Level& lvl,
                              int sx, int sy,
                              const std::pair<int,int>& target);
};
