#include "ControllerRobot.hpp"

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

void ControllerRobot::execute(const Command& cmd, WorldView& w) {
    if (!state->alive) return;
    if (cmd.type != CommandType::Broadcast) return;

    auto [dx, dy] = delta(cmd.dir);
    int fx = state->x + dx;
    int fy = state->y + dy;
    if (!inBounds(fx, fy, w.width, w.height)) return;

    for (auto& r : *w.robots) {
        if (!r.alive || r.type != RobotType::Worker) continue;
        if (r.x == fx && r.y == fy) {
            // Тут можна реалізувати логіку трансляції команди
            // Наприклад: позначити робота для наступної дії
        }
    }
}