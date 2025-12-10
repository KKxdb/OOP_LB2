#pragma once
#include "Robot.hpp"

class ControllerRobot : public Robot {
public:
    using Robot::Robot;

    void execute(const Command& cmd, WorldView& world) override;
};