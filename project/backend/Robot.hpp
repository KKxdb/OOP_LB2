#pragma once
#include "Types.hpp"
#include <optional>

// Базовий клас роботів.
// Робот зберігає тимчасові x/y (pending) до attachState() — так ми можемо setPosition() ще до того,
// як Level збереже RobotState.
class Robot {
protected:
    RobotState* state = nullptr;
    int pending_x = 0;
    int pending_y = 0;
public:
    Robot() = default;
    virtual ~Robot() = default;

    // attach state pointer (Level викликає після створення RobotState)
    void attachState(RobotState* s) { state = s; }
    RobotState* getState() { return state; }

    // виклики до позиції можна робити заздалегідь
    void setPosition(int x, int y) { pending_x = x; pending_y = y; }
    int getPendingX() const { return pending_x; }
    int getPendingY() const { return pending_y; }

    // тип робота (перевизначаються в похідних)
    virtual RobotType getType() const = 0;

    // виконати команду (переоприділяється)
    virtual void execute(const Command& cmd, WorldView& world) = 0;
};
