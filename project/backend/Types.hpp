#pragma once
#include <vector>
#include <optional>
#include <string>

// Базові типи клітинок
enum class CellType { Empty, Wall, Target };

enum class RobotType { Worker, Controller };

enum class CommandType { Move, Pick, Drop, Give, Broadcast };

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
};

// 🔧 Ось саме той WorldView, якого вам бракує
struct WorldView {
    int width;
    int height;
    const std::vector<std::vector<Cell>>* grid;
    std::vector<RobotState>* robots;
    std::vector<Box>* boxes;
};