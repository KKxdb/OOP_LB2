#pragma once
#include "Robot.hpp"

class ControllerRobot : public Robot {
public:
    ControllerRobot() = default;
    RobotType getType() const override { return RobotType::Controller; }
    void execute(const Command& cmd, WorldView& world) override;
};
