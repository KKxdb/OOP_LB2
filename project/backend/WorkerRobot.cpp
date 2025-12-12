#include "WorkerRobot.hpp"
#include <algorithm>
#include <cmath>
#include "Level.hpp"

static std::pair<int,int> delta(Direction d) {
    switch (d) {
        case Direction::Up: return {0,-1};
        case Direction::Down: return {0,1};
        case Direction::Left: return {-1,0};
        case Direction::Right: return {1,0};
    }
    return {0,0};
}

static bool inBounds(int x, int y, int w, int h) {
    return x >= 0 && y >= 0 && x < w && y < h;
}

static Box* findBoxById(std::vector<Box>& boxes, int id) {
    for (auto& b : boxes) if (b.id == id) return &b;
    return nullptr;
}

void WorkerRobot::execute(const Command& cmd, WorldView& w)
{
    RobotState* st = getState();
    if (!st || !st->alive) return;

    switch (cmd.type)
    {
        case CommandType::Move:
        {
            auto [dx, dy] = delta(cmd.dir);
            int nx = st->x + dx;
            int ny = st->y + dy;

            // Оновлюємо напрямок для правильних стрілок
            st->dir = cmd.dir;

            // 1) Вихід за межі → робот зникає
            if (nx < 0 || ny < 0 || nx >= w.width || ny >= w.height) {
                st->alive = false;
                st->carrying = false;
                st->boxId.reset();
                return;
            }

            // 2) Вхід у стіну → робот зникає
            if ((*w.grid)[ny][nx].type == CellType::Wall) {
                st->alive = false;
                st->carrying = false;
                st->boxId.reset();
                return;
            }

            // 3) Переносимо коробку, якщо робочий її несе
            if (st->carrying && st->boxId) {
                for (auto& b : *w.boxes) {
                    if (b.id == *st->boxId) {
                        b.x = nx;
                        b.y = ny;
                        break;
                    }
                }
            }

            // 4) Робітник рухається
            st->x = nx;
            st->y = ny;
            break;
        }

        case CommandType::Pick:
        {
            if (st->carrying) return;

            for (auto& b : *w.boxes) {
                if (b.x == st->x && b.y == st->y && !b.delivered) {
                    st->carrying = true;
                    st->boxId = b.id;
                    return;
                }
            }
            break;
        }

        case CommandType::Drop:
        {
            if (!st->carrying || !st->boxId) return;

            for (auto& b : *w.boxes) {
                if (b.id == *st->boxId) {
                    b.x = st->x;
                    b.y = st->y;
                    break;
                }
            }

            st->carrying = false;
            st->boxId.reset();
            break;
        }

        case CommandType::RotateCW:
        {
            switch (st->dir) {
                case Direction::Up:    st->dir = Direction::Right; break;
                case Direction::Right: st->dir = Direction::Down;  break;
                case Direction::Down:  st->dir = Direction::Left;  break;
                case Direction::Left:  st->dir = Direction::Up;    break;
            }
            break;
        }

        case CommandType::RotateCCW:
        {
            switch (st->dir) {
                case Direction::Up:    st->dir = Direction::Left;  break;
                case Direction::Left:  st->dir = Direction::Down;  break;
                case Direction::Down:  st->dir = Direction::Right; break;
                case Direction::Right: st->dir = Direction::Up;    break;
            }
            break;
        }

        default:
            break;
    }
}