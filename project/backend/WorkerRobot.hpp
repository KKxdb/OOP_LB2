#pragma once
#include "Robot.hpp"

class WorkerRobot : public Robot {
public:
    WorkerRobot() = default;
    RobotType getType() const override { return RobotType::Worker; }
    void execute(const Command& cmd, WorldView& world) override;
};
