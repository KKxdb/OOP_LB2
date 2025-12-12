#include "GameEngine.hpp"
#include "WorkerRobot.hpp"
#include "ControllerRobot.hpp"
using json = nlohmann::json;

GameEngine::GameEngine() : level(10, 10) {}

void GameEngine::loadLevel(Level&& lvl) {
    level = std::move(lvl);
    level.getPlacedRobots().clear();
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

// створюємо робота у списку UI
void GameEngine::spawnPlacedRobot(int x, int y, const std::string& type) {
    std::unique_ptr<Robot> r;

    if (type == "worker") r = std::make_unique<WorkerRobot>();
    else                  r = std::make_unique<ControllerRobot>();

    r->setPosition(x, y);
    level.getPlacedRobots().push_back(std::move(r));
}

// переносимо роботів у Level
void GameEngine::commitPlacedRobots() {
    for (auto& r : level.getPlacedRobots())
        level.addRobot(std::move(r));

    level.getPlacedRobots().clear();
}

// очищаємо при виході
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

    st["robots"] = json::array();

    // роботи з level (основні)
    for (auto& r : level.getRobots()) {
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

    // роботи, що поставив гравець у цьому рівні
    for (auto& r : level.getPlacedRobots()) {
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


    json out;
    out["state"] = st;
    return out;
}

void GameEngine::update() {
    level.update();
}

// додати робота одразу в рівень (Level::addRobot)
void GameEngine::addRobot(std::unique_ptr<Robot> r) {
    level.addRobot(std::move(r));
}

// додати робота в список "placed" у движку (якщо у тебе є placedRobots у GameEngine)
void GameEngine::addPlacedRobot(std::unique_ptr<Robot> r) {
    level.addPlacedRobot(std::move(r));
}


