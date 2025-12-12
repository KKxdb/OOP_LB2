#pragma once

#include <vector>
#include <memory>
#include <utility>

#include "Types.hpp"
#include "Robot.hpp"

class Level {
public:
    Level(int w = 10, int h = 10);

    void addRobot(std::unique_ptr<Robot> r);
    void addPlacedRobot(std::unique_ptr<Robot> r);
    void clearPlacedRobots();

    void addWall(int x, int y);
    void addTarget(int x, int y);
    void addBox(int x, int y);

    bool isInside(int x, int y) const;
    bool isWall(int x, int y) const;
    bool isTarget(int x, int y) const;
    bool isBox(int x, int y) const;

    void update();
    bool isCompleted() const;

    std::vector<std::vector<char>> getGrid() const;
    std::vector<std::vector<Cell>>& getGridCells();
    const std::vector<std::vector<Cell>>& getGridCells() const;

    std::vector<std::unique_ptr<Robot>>& getRobots();
    const std::vector<std::unique_ptr<Robot>>& getRobots() const;
    std::vector<std::unique_ptr<Robot>>& getPlacedRobots();
    const std::vector<std::unique_ptr<Robot>>& getPlacedRobots() const;

    std::vector<std::unique_ptr<RobotState>>& getRobotStates();

    std::vector<Box>& getBoxes();
    const std::vector<Box>& getBoxes() const;

    const std::vector<std::pair<int,int>>& getWalls() const { return walls; }
    const std::vector<std::pair<int,int>>& getTargets() const { return targets; }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    int getMoves() const { return moves; }
    void incrementMoves() { ++moves; }

    const Cell& getCell(int x, int y) const;
    Cell& getCell(int x, int y);

private:
    int width, height;
    int moves = 0;

    std::vector<std::vector<char>> grid;
    std::vector<std::vector<Cell>> gridCells;

    std::vector<std::pair<int,int>> walls;
    std::vector<std::pair<int,int>> targets;
    std::vector<std::pair<int,int>> boxesPos;

    std::vector<Box> boxStates;

    std::vector<std::unique_ptr<Robot>> robots;
    std::vector<std::unique_ptr<Robot>> placedRobots;

    std::vector<std::unique_ptr<RobotState>> robotStates;

    void rebuildBackground();
    void rebuildCells();
};
