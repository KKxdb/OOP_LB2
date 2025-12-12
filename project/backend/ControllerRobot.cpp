#include "ControllerRobot.hpp"
#include <algorithm>

void ControllerRobot::setCommand(const Command& cmd) {
    pendingCommand = cmd; // зберігаємо команду, яку КОНТРОЛЕР застосує до іншого робота
}

void ControllerRobot::execute(const Command& cmd, WorldView& w)
{
    RobotState* st = getState();
    if (!st || !st->alive) return;

    switch (cmd.type)
    {
        case CommandType::RotateCW:
        case CommandType::RotateCCW:
        {
            for (auto& upr : *w.robotStates)
            {
                RobotState& r = *upr;

                if (r.id == cmd.robotId)   // цільовий робот
                {
                    if (!r.alive) break;

                    if (cmd.type == CommandType::RotateCW) {
                        switch (r.dir) {
                            case Direction::Up:    r.dir = Direction::Right; break;
                            case Direction::Right: r.dir = Direction::Down;  break;
                            case Direction::Down:  r.dir = Direction::Left;  break;
                            case Direction::Left:  r.dir = Direction::Up;    break;
                        }
                    } else {
                        switch (r.dir) {
                            case Direction::Up:    r.dir = Direction::Left;  break;
                            case Direction::Left:  r.dir = Direction::Down;  break;
                            case Direction::Down:  r.dir = Direction::Right; break;
                            case Direction::Right: r.dir = Direction::Up;    break;
                        }
                    }
                    break;
                }
            }
            break;
        }

        case CommandType::Boost:
        {
            for (auto& upr : *w.robotStates)
            {
                RobotState& r = *upr;

                if (r.id == cmd.robotId)  
                {
                    r.boosted = true;  
                }
            }
            break;
        }

        default:
            break;
    }
}
