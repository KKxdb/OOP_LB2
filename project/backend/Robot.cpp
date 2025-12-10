#include "Robot.hpp"
#include <algorithm>

static std::pair<int, int> delta(Direction d) {
    switch (d) {
        case Direction::Up: return {0, -1};
        case Direction::Down: return {0, 1};
        case Direction::Left: return {-1, 0};
        case Direction::Right: return {1, 0};
    }
    return {0, 0};
}

static bool inBounds(int x, int y, int w, int h) {
    return x >= 0 && y >= 0 && x < w && y < h;
}

static Box* findBoxById(std::vector<Box>& boxes, int id) {
    for (auto& b : boxes) if (b.id == id) return &b;
    return nullptr;
}

void WorkerRobot::execute(const Command& cmd, WorldView& w) {
    if (!state->alive) return;

    if (cmd.type == CommandType::Move) {
        auto [dx, dy] = delta(cmd.dir);
        int nx = state->x + dx;
        int ny = state->y + dy;
        if (!inBounds(nx, ny, w.width, w.height)) return;

        auto cellType = w.grid->at(ny).at(nx).type;
        if (cellType == CellType::Wall) {
            state->alive = false;
            state->carrying = false;
            state->boxId.reset();
            return;
        }

        state->x = nx;
        state->y = ny;

        if (cellType == CellType::Target && state->carrying && state->boxId) {
            if (auto* b = findBoxById(*w.boxes, *state->boxId)) {
                b->x = nx;
                b->y = ny;
                b->delivered = true;
                state->carrying = false;
                state->boxId.reset();
            }
        }

        if (state->carrying && state->boxId) {
            if (auto* b = findBoxById(*w.boxes, *state->boxId)) {
                b->x = state->x;
                b->y = state->y;
            }
        }
    }

    else if (cmd.type == CommandType::Pick) {
        if (state->carrying) return;
        for (auto& b : *w.boxes) {
            if (!b.delivered && b.x == state->x && b.y == state->y) {
                state->carrying = true;
                state->boxId = b.id;
                break;
            }
        }
    }

    else if (cmd.type == CommandType::Drop) {
        if (!state->carrying || !state->boxId) return;
        if (auto* b = findBoxById(*w.boxes, *state->boxId)) {
            b->x = state->x;
            b->y = state->y;
        }
        state->carrying = false;
        state->boxId.reset();
    }

    else if (cmd.type == CommandType::Give) {
        if (!state->carrying || !state->boxId) return;
        for (auto& r : *w.robots) {
            if (!r.alive || r.id == state->id || r.type != RobotType::Worker) continue;
            int dx = std::abs(r.x - state->x);
            int dy = std::abs(r.y - state->y);
            if (dx + dy == 1 && !r.carrying) {
                r.carrying = true;
                r.boxId = state->boxId;
                state->carrying = false;
                state->boxId.reset();
                break;
            }
        }
    }
}

void ControllerRobot::execute(const Command& cmd, WorldView& w) {
    if (!state->alive || cmd.type != CommandType::Broadcast) return;

    auto [dx, dy] = delta(cmd.dir);
    int fx = state->x + dx;
    int fy = state->y + dy;
    if (!inBounds(fx, fy, w.width, w.height)) return;

    for (auto& r : *w.robots) {
        if (!r.alive || r.type != RobotType::Worker) continue;
        if (r.x == fx && r.y == fy) {
            // Можна реалізувати додаткову логіку тут
        }
    }
}