#pragma once
#include "Types.hpp"
#include <optional>

class Robot {
protected:
    RobotState* state = nullptr;
    int pending_x = 0;
    int pending_y = 0;
    Direction pending_dir = Direction::Up;
public:
    Robot() = default;
    virtual ~Robot() = default;

    void attachState(RobotState* s) { state = s; }
    RobotState* getState() { return state; }

    void setPosition(int x, int y) { pending_x = x; pending_y = y; }
    int getPendingX() const { return pending_x; }
    int getPendingY() const { return pending_y; }

    void setDirection(Direction d) { pending_dir = d; }
    Direction getDirection() const { return pending_dir; }

    virtual RobotType getType() const = 0;
    virtual void execute(const Command& cmd, WorldView& world) = 0;
};
