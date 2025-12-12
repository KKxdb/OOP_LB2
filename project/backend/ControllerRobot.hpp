#pragma once
#include "Robot.hpp"
#include <optional>

class ControllerRobot : public Robot {
public:
    ControllerRobot() = default;

    RobotType getType() const override { return RobotType::Controller; }

    // ← СИГНАТУРА №1 — має збігатися з cpp
    void execute(const Command& cmd, WorldView& w) override;

    // ← СИГНАТУРА №2 — також у cpp
    void setCommand(const Command& cmd);

private:
    std::optional<Command> pendingCommand;
};
