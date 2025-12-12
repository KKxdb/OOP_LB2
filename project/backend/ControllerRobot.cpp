#include "ControllerRobot.hpp"
#include <algorithm>

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
    if (!state || !state->alive) return;
    if (cmd.type != CommandType::Broadcast) return;

    auto [dx, dy] = delta(cmd.dir);
    int fx = state->x + dx;
    int fy = state->y + dy;
    if (!inBounds(fx, fy, w.width, w.height)) return;

    // просто сигналізуємо сусідньому роботу (фактично тут можна додати
    // якусь поведінку; зараз нічого не змінюємо — контролер просто "бродкаст")
    for (auto& r : *w.robots) {
        if (!r.alive || r.type != RobotType::Worker) continue;
        if (r.x == fx && r.y == fy) {
            // наприклад, позначити: r.carrying = true;  але це логіка гри — залишаю пустим
        }
    }
}
