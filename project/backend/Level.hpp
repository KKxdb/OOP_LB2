#pragma once
#include <memory>
#include <vector>
#include <utility>
#include <algorithm>
#include "Robot.hpp"
#include "Types.hpp"

// Level: зберігає робітників, стани, коробки, цілі, стіни
class Level {
public:
    Level(int w = 10, int h = 10);

    void addRobot(std::unique_ptr<Robot> r); // attach state та додати робота
    void addWall(int x, int y);
    void addTarget(int x, int y);
    void addBox(int x, int y);

    void update();

    // доступи
    std::vector<std::vector<char>> getGrid() const;
    std::vector<std::vector<Cell>>& getGridCells();
    const std::vector<std::vector<Cell>>& getGridCells() const;

    const std::vector<std::unique_ptr<Robot>>& getRobots() const;
    std::vector<std::unique_ptr<Robot>>& getRobots();

    std::vector<RobotState>& getRobotStates();
    std::vector<Box>& getBoxes();
    const std::vector<Box>& getBoxes() const;

    const std::vector<std::pair<int,int>>& getTargets() const { return targets; }
    const std::vector<std::pair<int,int>>& getWalls() const { return walls; }

    bool isInside(int x, int y) const;
    bool isWall(int x, int y) const;
    bool isTarget(int x, int y) const;
    bool isBox(int x, int y) const;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    bool isCompleted() const;

    int getMoves() const { return moves; }       
    void incrementMoves() { moves++; }

private:
    int width;
    int height;

    int moves = 0;

    std::vector<std::vector<char>> grid;
    std::vector<std::vector<Cell>> gridCells;

    std::vector<std::pair<int,int>> walls;
    std::vector<std::pair<int,int>> targets;
    std::vector<std::pair<int,int>> boxes;

    std::vector<std::unique_ptr<Robot>> robots;
    std::vector<RobotState> robotStates;
    std::vector<Box> boxStates;

    void rebuildBackground();
    void rebuildCells();
};
