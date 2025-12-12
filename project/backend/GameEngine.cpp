#include "GameEngine.hpp"
#include <nlohmann/json.hpp>
#include "Level.hpp"


using json = nlohmann::json;

GameEngine::GameEngine() : level(10,10) {}

void GameEngine::loadLevel(Level&& lvl) {
    level = std::move(lvl);
}

Robot* GameEngine::findRobotById(int id) {
    for (auto& r : level.getRobots()) {
        auto* st = r->getState();
        if (st && st->id == id) return r.get();
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

    // walls
    st["walls"] = json::array();
    for (auto& w : level.getWalls()) {
        st["walls"].push_back({ {"x", w.first}, {"y", w.second} });
    }

    // targets
    st["targets"] = json::array();
    for (auto& t : level.getTargets()) {
        st["targets"].push_back({ {"x", t.first}, {"y", t.second} });
    }

    // boxes
    st["boxes"] = json::array();
    for (auto& b : level.getBoxes()) {
        st["boxes"].push_back({ {"x", b.x}, {"y", b.y} });
    }

    // robots
    st["robots"] = json::array();
    for (auto& r : level.getRobots()) {
        auto* rs = r->getState();
        if (!rs) continue;

        st["robots"].push_back({
            {"x", rs->x},
            {"y", rs->y},
            {"id", rs->id},
            {"type", rs->type == RobotType::Worker ? "worker" : "controller"}
        });
    }

    // упаковка
    json out;
    out["state"] = st;
    return out;
}
