#pragma once
#include "Robot.hpp"

class WorkerRobot : public Robot {
public:
    using Robot::Robot;

    void execute(const Command& cmd, WorldView& world) override;
};