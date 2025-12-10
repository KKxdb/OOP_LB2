#pragma once
#include "Types.hpp"

// Базовий клас для всіх роботів
class Robot {
protected:
    RobotState* state;

public:
    explicit Robot(RobotState* s) : state(s) {}
    virtual ~Robot() = default;

    RobotState* getState() { return state; }

    virtual void execute(const Command& cmd, WorldView& world) = 0;

    // Для рендеру
    int getX() const { return state->x; }
    int getY() const { return state->y; }
    char getSymbol() const {
        if (!state->alive) return 'X';
        if (state->type == RobotType::Worker)
            return state->carrying ? 'B' : 'W';
        else
            return 'C';
    }
};

// Робот-робітник
class WorkerRobot : public Robot {
public:
    using Robot::Robot;
    void execute(const Command& cmd, WorldView& world) override;
};

// Робот-контролер
class ControllerRobot : public Robot {
public:
    using Robot::Robot;
    void execute(const Command& cmd, WorldView& world) override;
};