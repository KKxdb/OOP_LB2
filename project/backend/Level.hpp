#pragma once
#include <memory>
#include <vector>
#include <utility>
#include "Robot.hpp"
#include "Types.hpp"

// Рівень: фіксована сітка, стіни, цілі, коробки, роботи
class Level {
public:
    Level(int w, int h);

    void addRobot(std::unique_ptr<Robot> r);
    void addWall(int x, int y);
    void addTarget(int x, int y);
    void addBox(int x, int y);

    void update();

    // фонова сітка (символи)
    std::vector<std::vector<char>> getGrid() const;

    // логічна сітка (Cell)
    std::vector<std::vector<Cell>>& getGridCells();
    const std::vector<std::vector<Cell>>& getGridCells() const;

    // доступ до роботів
    const std::vector<std::unique_ptr<Robot>>& getRobots() const;
    std::vector<std::unique_ptr<Robot>>& getRobots();

    // доступ до станів і коробок
    std::vector<RobotState>& getRobotStates();
    std::vector<Box>& getBoxes();
    const std::vector<Box>& getBoxes() const;


    // перевірки
    bool isInside(int x, int y) const;
    bool isWall(int x, int y) const;
    bool isTarget(int x, int y) const;
    bool isBox(int x, int y) const;

    // розміри
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    bool isCompleted() const;

private:
    int width;
    int height;

    std::vector<std::vector<char>> grid;       // для рендера
    std::vector<std::vector<Cell>> gridCells;  // для логіки

    std::vector<std::pair<int,int>> walls;
    std::vector<std::pair<int,int>> targets;
    std::vector<std::pair<int,int>> boxes;

    std::vector<std::unique_ptr<Robot>> robots;
    std::vector<RobotState> robotStates;
    std::vector<Box> boxStates;

    void rebuildBackground();
    void rebuildCells();
};