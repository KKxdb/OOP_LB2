#pragma once
#include <vector>
#include <optional>
#include <memory>
#include <string>

enum class CellType { Empty, Wall, Target };
enum class RobotType { Worker, Controller };
enum class CommandType { Move, Pick, Drop, Give, Broadcast, RotateCW, RotateCCW, Boost };
enum class Direction { Up, Down, Left, Right };

struct Command {
    int robotId;
    CommandType type;
    Direction dir;
};

struct Cell {
    CellType type = CellType::Empty;
};

struct Box {
    int id;
    int x;
    int y;
    bool delivered = false;
};

struct RobotState {
    int id;
    RobotType type;
    int x;
    int y;
    bool alive = true;
    bool carrying = false;
    std::optional<int> boxId;
    Direction dir = Direction::Up;
    bool boosted = false;
    
};

// правильний WorldView
struct WorldView {
    int width;
    int height;

    const std::vector<std::vector<Cell>>* grid;
    std::vector<std::unique_ptr<RobotState>>* robotStates;
    std::vector<Box>* boxes;
};
