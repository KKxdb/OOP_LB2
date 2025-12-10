#include "GameEngine.hpp"
#include "WorkerRobot.hpp"
#include "ControllerRobot.hpp"
#include "Level.hpp"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

GameEngine::GameEngine()
    : level(10, 10) {}

std::string GameEngine::processCommand(const std::string& data) {
    json cmd = json::parse(data);
    const std::string action = cmd.value("action", "");

    if (action == "add_worker") {
        int x = cmd.value("x", 0);
        int y = cmd.value("y", 0);
        RobotState rs{(int)level.getRobots().size()+1, RobotType::Worker, x, y};
        level.addRobot(std::make_unique<WorkerRobot>(&rs));
    } else if (action == "add_controller") {
        int x = cmd.value("x", 0);
        int y = cmd.value("y", 0);
        RobotState rs{(int)level.getRobots().size()+1, RobotType::Controller, x, y};
        level.addRobot(std::make_unique<ControllerRobot>(&rs));
    } else if (action == "add_wall") {
        level.addWall(cmd.value("x", 0), cmd.value("y", 0));
    } else if (action == "add_target") {
        level.addTarget(cmd.value("x", 0), cmd.value("y", 0));
    } else if (action == "add_box") {
        level.addBox(cmd.value("x", 0), cmd.value("y", 0));
    } else if (action == "step") {
        std::vector<Command> cmds;
        if (cmd.contains("commands")) {
            for (auto& jc : cmd["commands"]) {
                Command c;
                c.robotId = jc.value("robot_id", 0);
                std::string type = jc.value("cmd", "");
                if (type == "move") c.type = CommandType::Move;
                else if (type == "pick") c.type = CommandType::Pick;
                else if (type == "drop") c.type = CommandType::Drop;
                else if (type == "give") c.type = CommandType::Give;
                else c.type = CommandType::Broadcast;

                std::string dir = jc.value("dir", "");
                if (dir == "up") c.dir = Direction::Up;
                else if (dir == "down") c.dir = Direction::Down;
                else if (dir == "left") c.dir = Direction::Left;
                else c.dir = Direction::Right;

                cmds.push_back(c);
            }
        }

        WorldView view{level.getWidth(), level.getHeight(),
                       &level.getGridCells(), &level.getRobotStates(), &level.getBoxes()};

        for (auto& c : cmds) {
            for (auto& r : level.getRobots()) {
                if (r->getState()->id == c.robotId) {
                    r->execute(c, view);
                }
            }
        }

        level.update();
    }

    json response;
    response["grid_w"] = level.getWidth();
    response["grid_h"] = level.getHeight();
    response["grid"] = level.getGrid();

    json robots = json::array();
    for (auto& r : level.getRobots()) {
        auto* st = r->getState();
        json rr;
        rr["id"] = st->id;
        rr["type"] = (st->type == RobotType::Worker) ? "worker" : "controller";
        rr["x"] = st->x;
        rr["y"] = st->y;
        rr["alive"] = st->alive;
        rr["carrying"] = st->carrying;
        rr["box_id"] = st->boxId.has_value() ? *st->boxId : 0;
        robots.push_back(rr);
    }
    response["robots"] = robots;

    json boxes = json::array();
    for (const auto& b : level.getBoxes()) {
        json bb;
        bb["id"] = b.id;
        bb["x"] = b.x;
        bb["y"] = b.y;
        bb["delivered"] = b.delivered;
        boxes.push_back(bb);
    }
    response["boxes"] = boxes;

    response["completed"] = level.isCompleted();
    return response.dump();
}

void GameEngine::loadLevel(Level&& lvl) {
    level = std::move(lvl);
}

Robot* GameEngine::findRobotById(int id) {
    for (auto& r : level.getRobots()) {
        if (r->getState()->id == id) return r.get();
    }
    return nullptr;
}

void GameEngine::applyCommands(const std::vector<Command>& cmds) {
    WorldView view{level.getWidth(), level.getHeight(),
                   &level.getGridCells(), &level.getRobotStates(), &level.getBoxes()};

    for (const auto& c : cmds) {
        Robot* r = findRobotById(c.robotId);
        if (r) r->execute(c, view);
    }

    level.update();
}

nlohmann::json GameEngine::getStateJson() const {
    nlohmann::json j;
    j["grid_w"] = level.getWidth();
    j["grid_h"] = level.getHeight();
    j["grid"] = level.getGrid();

    nlohmann::json robots = nlohmann::json::array();
    for (const auto& r : level.getRobots()) {
        auto* st = r->getState();
        nlohmann::json rr;
        rr["id"] = st->id;
        rr["type"] = (st->type == RobotType::Worker) ? "worker" : "controller";
        rr["x"] = st->x;
        rr["y"] = st->y;
        rr["alive"] = st->alive;
        rr["carrying"] = st->carrying;
        rr["box_id"] = st->boxId.has_value() ? *st->boxId : 0;
        robots.push_back(rr);
    }
    j["robots"] = robots;

    nlohmann::json boxes = nlohmann::json::array();
    for (const auto& b : level.getBoxes()) {
        nlohmann::json bb;
        bb["id"] = b.id;
        bb["x"] = b.x;
        bb["y"] = b.y;
        bb["delivered"] = b.delivered;
        boxes.push_back(bb);
    }
    j["boxes"] = boxes;

    j["completed"] = level.isCompleted();
    return j;
}