#include "GameEngine.hpp"
#include "WorkerRobot.hpp"
#include "ControllerRobot.hpp"
#include <queue>

using json = nlohmann::json;

GameEngine::GameEngine() : level(10, 10) {}
bool running_= false;

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
            if (!s || !s->alive) continue;

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

    return {
    {"state", st},
    {"finished", isWin() || isLose()},
    {"win", isWin()}
};
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
    running_ = true;
    if (locked_) return;

    auto& robots = level.getRobots();
    auto& placed = level.getPlacedRobots();

    // ===== КОНТРОЛЕРИ =====
    auto controllerAction = [&](Robot* base){
        ControllerRobot* c = dynamic_cast<ControllerRobot*>(base);
        if (!c) return;

        auto* sc = c->getState();
        if (!sc || !c->hasPendingCommand()) return;

        int dx = 0, dy = 0;
        switch (sc->dir) {
            case Direction::Up:    dy = -1; break;
            case Direction::Down:  dy = 1; break;
            case Direction::Left:  dx = -1; break;
            case Direction::Right: dx = 1; break;
        }

        int tx = sc->x + dx;
        int ty = sc->y + dy;

        auto apply = [&](auto& arr){
            for (auto& r : arr) {
                auto* s = r->getState();
                if (!s) continue;

                if (s->x == tx && s->y == ty) {
                    Command cmd = c->takePendingCommand();

                    WorldView view{
                        level.getWidth(),
                        level.getHeight(),
                        &level.getGridCells(),
                        &level.getRobotStates(),
                        &level.getBoxes()
                    };

                    r->execute(cmd, view);
                    return;
                }
            }
        };

        apply(robots);
        apply(placed);
    };

    for (auto& r : robots) controllerAction(r.get());
    for (auto& r : placed) controllerAction(r.get());


    // ===== РУХ WORKER =====
    auto moveWorker = [&](Robot* base) {
        WorkerRobot* w = dynamic_cast<WorkerRobot*>(base);
        if (!w) return;

        auto* s = w->getState();
        if (!s || !s->alive) return;

        int dx = 0, dy = 0;
        switch (s->dir) {
            case Direction::Up:    dy = -1; break;
            case Direction::Down:  dy = 1; break;
            case Direction::Left:  dx = -1; break;
            case Direction::Right: dx = 1; break;
        }

        int nx = s->x + dx;
        int ny = s->y + dy;

        // 1) вихід за межі
        if (!level.isInside(nx, ny)) {
            s->alive = false;
            return;
        }

        // 2) стіна
        if (level.getCell(nx, ny).type == CellType::Wall) {
            s->alive = false;
            return;
        }

        // 3) зіткнення з роботами
        auto collide = [&](auto& arr) {
            for (auto& r : arr) {
                auto* o = r->getState();
                if (!o || o == s) continue;
                if (o->x == nx && o->y == ny) return true;
            }
            return false;
        };

        if (collide(robots) || collide(placed)) {
            s->alive = false;
            return;
        }

        // 4) рух
        s->x = nx;
        s->y = ny;

        // 5) підбір коробки
        if (!s->carrying) {
            for (auto& b : level.getBoxes()) {
                if (!b.delivered && b.x == nx && b.y == ny) {
                    s->carrying = true;
                    s->boxId = b.id;
                    break;
                }
            }
        }

        // 6) рух коробки + здача
        if (s->carrying && s->boxId.has_value()) {
            int bid = *s->boxId;
            for (auto& b : level.getBoxes()) {
                if (b.id == bid) {
                    b.x = nx;
                    b.y = ny;

                    for (auto& t : level.getTargets()) {
                        if (t.first == b.x && t.second == b.y) {
                            b.delivered = true;
                            s->carrying = false;
                            s->boxId.reset();
                        }
                    }
                    break;
                }
            }
        }
    };

    // ===== ЗАПУСК РУХУ WORKER =====
    for (auto& r : robots) moveWorker(r.get());
    for (auto& r : placed) moveWorker(r.get());

    // ===== ВИДАЛЕННЯ МЕРТВИХ =====
    robots.erase(
        std::remove_if(robots.begin(), robots.end(),
            [](const std::unique_ptr<Robot>& r) {
                auto* s = r->getState();
                return s && !s->alive;
            }),
        robots.end()
    );

    placed.erase(
        std::remove_if(placed.begin(), placed.end(),
            [](const std::unique_ptr<Robot>& r) {
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
    // якщо гра ще не запущена — поразки бути не може
    if (!running_) 
        return false;

    auto hasAliveWorker = [&](const auto& robots) {
        for (const auto& r : robots) {
            auto* s = r->getState();
            if (!s) continue;

            if (s->alive && s->type == RobotType::Worker)
                return true;
        }
        return false;
    };

    // перевіряємо всі реальні роботи
    if (hasAliveWorker(level.getRobots()))
        return false;

    if (hasAliveWorker(level.getPlacedRobots()))
        return false;

    // рух іде, але немає жодного живого Worker — поразка
    return true;
}

