#pragma once
#include "Robot.hpp"
#include <optional>

class ControllerRobot : public Robot {
public:
    ControllerRobot() = default;

    RobotType getType() const override { return RobotType::Controller; }

    void execute(const Command& cmd, WorldView& w) override;

    void setCommand(const Command& cmd);

    bool hasPendingCommand() const { return pendingCommand.has_value(); }
    Command takePendingCommand() {
        Command c = pendingCommand.value();
        pendingCommand.reset();
        return c;
    }

private:
    std::optional<Command> pendingCommand;
};
