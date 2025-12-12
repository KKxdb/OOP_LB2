#include "ControllerRobot.hpp"
#include <algorithm>
#include <cmath>

static std::pair<int,int> delta(Direction d) {
    switch (d) {
        case Direction::Up:    return {0,-1};
        case Direction::Down:  return {0,1};
        case Direction::Left:  return {-1,0};
        case Direction::Right: return {1,0};
    }
    return {0,0};
}

static bool inBounds(int x, int y, int w, int h) {
    return x >= 0 && y >= 0 && x < w && y < h;
}

void ControllerRobot::setCommand(const Command& cmd) {
    pendingCommand = cmd;     // просто зберігаємо майбутню команду
}

void ControllerRobot::execute(const Command& cmdIncoming, WorldView& w) {
    if (!state || !state->alive)
        return;

    // беремо збережену команду або поточну
    Command cmd = pendingCommand.value_or(cmdIncoming);
    pendingCommand.reset();

    switch (cmd.type) {

        // ================= ROTATE CLOCKWISE =================
        case CommandType::RotateCW:
        {
            switch (state->dir) {
                case Direction::Up:    state->dir = Direction::Right; break;
                case Direction::Right: state->dir = Direction::Down;  break;
                case Direction::Down:  state->dir = Direction::Left;  break;
                case Direction::Left:  state->dir = Direction::Up;    break;
            }
            return;
        }

        // =============== ROTATE COUNTER-CLOCKWISE =================
        case CommandType::RotateCCW:
        {
            switch (state->dir) {
                case Direction::Up:    state->dir = Direction::Left;  break;
                case Direction::Left:  state->dir = Direction::Down;  break;
                case Direction::Down:  state->dir = Direction::Right; break;
                case Direction::Right: state->dir = Direction::Up;    break;
            }
            return;
        }

        // ======================= BOOST ==========================
        case CommandType::Boost:
        {
            auto [dx, dy] = delta(state->dir);

            int x1 = state->x + dx;
            int y1 = state->y + dy;
            int x2 = state->x + 2*dx;
            int y2 = state->y + 2*dy;

            if (!inBounds(x2, y2, w.width, w.height)) return;
            if ((*w.grid)[y1][x1].type == CellType::Wall) return;
            if ((*w.grid)[y2][x2].type == CellType::Wall) return;

            state->x = x2;
            state->y = y2;
            return;
        }

        // ===================== BROADCAST ===========================
        case CommandType::Broadcast:
        {
            state->dir = cmd.dir;

            auto [dx, dy] = delta(cmd.dir);
            int fx = state->x + dx;
            int fy = state->y + dy;

            if (!inBounds(fx, fy, w.width, w.height))
                return;

            // сигнал worker-ам
            for (auto& upr : *w.robotStates) {
                RobotState& r = *upr;

                if (!r.alive || r.id == state->id) continue;
                if (r.type != RobotType::Worker) continue;

                int ddx = std::abs(r.x - state->x);
                int ddy = std::abs(r.y - state->y);

                // тут можна додати реакцію Worker
            }

            return;
        }

        default:
            return;
    }
}
