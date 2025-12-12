#include "RequestHandler.hpp"
#include "GameEngine.hpp"
#include "LevelLoader.hpp"
#include "WorkerRobot.hpp"
#include "ControllerRobot.hpp"

#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

RequestHandler::RequestHandler(GameEngine& engine) : eng_(engine) {}

json RequestHandler::handle(const json& req) {
    std::string action = req.value("action", "");

    // ----------------- LOAD LEVEL -----------------
    if (action == "load_level") {
        try {
            std::string path = req.at("path").get<std::string>();
            std::cerr << "[RequestHandler] load_level path=" << path << std::endl;

            auto opt = LevelLoader::loadFromJson(path);
            if (!opt)
                return json{{"status","error"},{"message","cannot load level"}};

            eng_.loadLevel(std::move(*opt));
        }
        catch (std::exception& e) {
            return json{{"status","error"},{"message", e.what()}};
        }

        auto st = eng_.getStateJson();
        return json{{"status","ok"},{"state", st["state"]}};
    }

    // ----------------- STATUS -----------------
    if (action == "status") {
        auto st = eng_.getStateJson();
        return json{{"status","ok"},{"state", st["state"]}};
    }

    // ----------------- ADD ROBOT -----------------
    if (action == "add_robot") {
        try {
            std::string type = req.value("robot_type", "worker");
            int x = req.value("x", 0);
            int y = req.value("y", 0);

            std::unique_ptr<Robot> r;

            if (type == "worker")
                r = std::make_unique<WorkerRobot>();
            else
                r = std::make_unique<ControllerRobot>();

            r->setPosition(x, y);
            eng_.addRobot(std::move(r));

            auto st = eng_.getStateJson();
            return json{{"status","ok"},{"state", st["state"]}};
        }
        catch (...) {
            return json{{"status","error"},{"message","cannot add robot"}};
        }
    }

    // ----------------- STEP -----------------
    if (action == "step") {

        std::vector<Command> cmds;
        if (req.contains("commands")) {
            for (auto &c : req["commands"]) {
                Command cmd;
                cmd.robotId = c.value("robot_id", -1);

                std::string t = c.value("cmd","");
                if (t == "move") cmd.type = CommandType::Move;
                else if (t == "pick") cmd.type = CommandType::Pick;
                else if (t == "drop") cmd.type = CommandType::Drop;
                else if (t == "give") cmd.type = CommandType::Give;
                else cmd.type = CommandType::Broadcast;

                std::string dir = c.value("dir","");
                if (dir == "up") cmd.dir = Direction::Up;
                else if (dir == "down") cmd.dir = Direction::Down;
                else if (dir == "left") cmd.dir = Direction::Left;
                else cmd.dir = Direction::Right;

                cmds.push_back(cmd);
            }
        }

        eng_.applyCommands(cmds);
        auto st = eng_.getStateJson();
        return json{{"status","ok"},{"state", st["state"]}};
    }

    return json{{"status","error"},{"message","unknown action"}};
}
