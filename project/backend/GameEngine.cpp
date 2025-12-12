#include "GameEngine.hpp"
#include "WorkerRobot.hpp"
#include "ControllerRobot.hpp"
#include <queue>

using json = nlohmann::json;

GameEngine::GameEngine() : level(10, 10) {}

void GameEngine::loadLevel(Level&& lvl) {
    level = std::move(lvl);
}

static std::string dirToStr(Direction d) {
    switch (d) {
        case Direction::Up: return "up";
        case Direction::Down: return "down";
        case Direction::Left: return "left";
        case Direction::Right: return "right";
    }
    return "up";
}

void GameEngine::spawnPlacedRobot(int x, int y, const std::string& type) {
    std::unique_ptr<Robot> r;

    if (type == "worker")
        r = std::make_unique<WorkerRobot>();
    else
        r = std::make_unique<ControllerRobot>();

    r->setPosition(x, y);
    level.getPlacedRobots().push_back(std::move(r));
}
void GameEngine::commitPlacedRobots() {
    for (auto& r : level.getPlacedRobots())
        level.addRobot(std::move(r));
    level.getPlacedRobots().clear();
}

void GameEngine::clearPlacedRobots() {
    level.getPlacedRobots().clear();
}

Robot* GameEngine::findRobotById(int id) {
    for (auto& r : level.getRobots()) {
        if (r->getState() && r->getState()->id == id)
            return r.get();
    }
    return nullptr;
}

void GameEngine::applyCommands(const std::vector<Command>& cmds) {
    WorldView view{
        level.getWidth(),
        level.getHeight(),
        &level.getGridCells(),
        &level.getRobotStates(),
        &level.getBoxes()
    };

    for (auto& c : cmds) {
        Robot* r = findRobotById(c.robotId);
        if (r) r->execute(c, view);
    }

    level.incrementMoves();
    level.update();
}

json GameEngine::getStateJson() const {
    json st;

    st["width"]  = level.getWidth();
    st["height"] = level.getHeight();

    st["walls"] = json::array();
    for (auto& w : level.getWalls())
        st["walls"].push_back({ {"x", w.first}, {"y", w.second} });

    st["targets"] = json::array();
    for (auto& t : level.getTargets())
        st["targets"].push_back({ {"x", t.first}, {"y", t.second} });

    st["boxes"] = json::array();
    for (auto& b : level.getBoxes())
        st["boxes"].push_back({ {"x", b.x}, {"y", b.y} });

    // Один загальний список роботів
    st["robots"] = json::array();

    auto append = [&](const auto& arr){
        for (auto& r : arr) {
            auto* s = r->getState();
            if (!s) continue;

            st["robots"].push_back({
                {"id", s->id},
                {"x",  s->x},
                {"y",  s->y},
                {"type", s->type == RobotType::Worker ? "worker" : "controller"},
                {"dir", dirToStr(s->dir)}
            });
        }
    };

    append(level.getRobots());
    append(level.getPlacedRobots());

    return json{ {"state", st} };
}


void GameEngine::update() {
    level.update();
}

void GameEngine::addRobot(std::unique_ptr<Robot> r) {
    level.addRobot(std::move(r));
}

void GameEngine::addPlacedRobot(std::unique_ptr<Robot> r) {
    level.addPlacedRobot(std::move(r));
}

void GameEngine::stepAuto() {
    if (locked_) return;

    // Спільний контейнер — всі реальні роботи
    auto& robots = level.getRobots();
    auto& placed = level.getPlacedRobots();

    auto moveWorker = [&](Robot* base) {
    WorkerRobot* w = dynamic_cast<WorkerRobot*>(base);
    if (!w) return;

    auto* s = w->getState();
    if (!s) return;

    int dx = 0, dy = 0;
    switch (s->dir) {
        case Direction::Up:    dy = -1; break;
        case Direction::Down:  dy = 1; break;
        case Direction::Left:  dx = -1; break;
        case Direction::Right: dx = 1; break;
    }

    int nx = s->x + dx;
    int ny = s->y + dy;

    // === 1) Якщо виходить за межі — вбити робота ===
    if (!level.isInside(nx, ny)) {
        s->alive = false;
        return;
    }

    // === 2) Якщо ціль — стіна — теж вбити робота ===
    CellType ct = level.getCell(nx, ny).type;
    if (ct == CellType::Wall) {
        s->alive = false;
        return;
    }

    // Перевірка зіткнень з реальними роботами
    auto check = [&](auto& arr){
        for (auto& other : arr) {
            auto* so = other->getState();
            if (!so) continue;
            if (so != s && so->x == nx && so->y == ny)
                return true;
        }
        return false;
    };

    if (check(robots)) return;
    if (check(placed)) return;

    // === 3) Перемістити робота ===
    s->x = nx;
    s->y = ny;

    // === 4) Спроба підняти коробку ===
    // якщо робот нічого не несе
    if (!s->carrying) {
        for (auto& b : level.getBoxes()) {
            if (!b.delivered && b.x == nx && b.y == ny) {
                s->carrying = true;
                s->boxId = b.id;
                break;
            }
        }
    }

    // === 5) Якщо несе коробку — коробка рухається разом з ним ===
    if (s->carrying && s->boxId.has_value()) {
        int bid = *s->boxId;
        for (auto& b : level.getBoxes()) {
            if (b.id == bid) {
                b.x = nx;
                b.y = ny;
            }
        }
    }
};

    auto controllerAction = [&](Robot* base){
    ControllerRobot* c = dynamic_cast<ControllerRobot*>(base);
    if (!c) return;

    auto* sc = c->getState();
    if (!sc) return;

    int dx = 0, dy = 0;
    switch (sc->dir) {
        case Direction::Up:    dy = -1; break;
        case Direction::Down:  dy = 1; break;
        case Direction::Left:  dx = -1; break;
        case Direction::Right: dx = 1; break;
    }

    int tx = sc->x + dx;
    int ty = sc->y + dy;

    bool acted = false;

    auto applyTo = [&](auto& arr){
        for (auto& other : arr) {
            auto* so = other->getState();
            if (!so) continue;

            if (so->x == tx && so->y == ty) {

                if (!c->hasPendingCommand()) return;

                Command cmd = c->takePendingCommand();

                WorldView view{
                    level.getWidth(),
                    level.getHeight(),
                    &level.getGridCells(),
                    &level.getRobotStates(),
                    &level.getBoxes()
                };

                other->execute(cmd, view);
                acted = true;
                return;
            }
        }
    };

    applyTo(robots);
    if (!acted) applyTo(placed);
};

    // 2) Робота контролерів
    for (auto& r : robots) controllerAction(r.get());
    for (auto& r : placed) controllerAction(r.get());

    auto& robots = level.getRobots();
    robots.erase(
        std::remove_if(robots.begin(), robots.end(),
                    [](const std::unique_ptr<Robot>& r){
                        auto* s = r->getState();
                        return s && !s->alive;
                    }),
        robots.end()
    );

    auto& placed = level.getPlacedRobots();
    placed.erase(
        std::remove_if(placed.begin(), placed.end(),
                    [](const std::unique_ptr<Robot>& r){
                        auto* s = r->getState();
                        return s && !s->alive;
                    }),
        placed.end()
    );

    level.update();
}




std::vector<Command> GameEngine::collectControllerCommands() {
    std::vector<Command> cmds;

    for (auto& r : level.getRobots()) {
        auto* ctrl = dynamic_cast<ControllerRobot*>(r.get());
        if (!ctrl) continue;

        if (ctrl->hasPendingCommand()) {
            cmds.push_back(ctrl->takePendingCommand());
        }
    }

    return cmds;
}

bool GameEngine::isWin() const {
    return level.isCompleted();
}

bool GameEngine::bfs_reachable(
    const Level& lvl,
    int sx, int sy,
    const std::pair<int,int>& target)
{
    int W = lvl.getWidth();
    int H = lvl.getHeight();

    if (!lvl.isInside(sx, sy)) return false;

    std::vector<std::vector<char>> used(H, std::vector<char>(W, 0));
    std::queue<std::pair<int,int>> q;
    q.push({sx, sy});
    used[sy][sx] = 1;

    const int dx[4] = {1,-1,0,0};
    const int dy[4] = {0,0,1,-1};

    while (!q.empty()) {
        auto [x, y] = q.front(); q.pop();

        if (x == target.first && y == target.second)
            return true;

        for (int k=0; k<4; ++k) {
            int nx = x + dx[k];
            int ny = y + dy[k];

            if (!lvl.isInside(nx, ny)) continue;
            if (used[ny][nx]) continue;
            if (lvl.isWall(nx, ny)) continue;

            used[ny][nx] = 1;
            q.push({nx, ny});
        }
    }

    return false;
}

bool GameEngine::isLose() const {
    const auto& boxes = level.getBoxes();
    const auto& targets = level.getTargets();

    bool anyUndelivered = false;
    for (const auto& b : boxes)
        if (!b.delivered) anyUndelivered = true;

    if (!anyUndelivered) return false; // можливо перемога

    if (targets.empty()) return true; // немає куди доставити

    for (const auto& b : boxes) {
        if (b.delivered) continue;

        for (const auto& t : targets)
            if (bfs_reachable(level, b.x, b.y, t))
                return false; // коробка може дістатися цілі
    }

    return true; // жодна коробка не може дістатись жодної цілі
}
