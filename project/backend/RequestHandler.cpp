#include "RequestHandler.hpp"
#include "GameEngine.hpp"
#include "LevelLoader.hpp"
#include "WorkerRobot.hpp"
#include "ControllerRobot.hpp"

#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

static Direction strToDir(const std::string& s) {
    if (s == "up" || s == "Up" || s == "UP") return Direction::Up;
    if (s == "down" || s == "Down" || s == "DOWN") return Direction::Down;
    if (s == "left" || s == "Left" || s == "LEFT") return Direction::Left;
    if (s == "right" || s == "Right" || s == "RIGHT") return Direction::Right;
    return Direction::Up;
}

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
        if (eng_.isLocked())
            return json{{"status","error"},{"message","robots_locked"}};
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
                else if (t == "rotate_cw") cmd.type = CommandType::RotateCW;
                else if (t == "rotate_ccw") cmd.type = CommandType::RotateCCW;
                else if (t == "boost") cmd.type = CommandType::Boost;
                else cmd.type = CommandType::Broadcast;  // default

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


    if (action == "place_robot") {
        if (eng_.isLocked())
        return json{{"status","error"},{"message","robots_locked"}};

        int x = req["x"];
        int y = req["y"];
        std::string type = req["robot_type"];
        std::string dir = req["direction"];

        std::unique_ptr<Robot> r;

        if (type == "worker") r = std::make_unique<WorkerRobot>();
        else r = std::make_unique<ControllerRobot>();

        r->setPosition(x, y);
        r->setDirection(strToDir(dir));

        eng_.addPlacedRobot(std::move(r));

        auto st = eng_.getStateJson();
        return json{{"status","ok"},{"state", st["state"]}};
    }

    if (action == "spawn_robot") {
    if (eng_.isLocked())
        return json{{"status","error"},{"message","robots_locked"}};

    try {
        std::string type = req.value("type", "worker");
        std::string dir  = req.value("dir", "up");

        int x = req.value("x", 0);
        int y = req.value("y", 0);

        std::unique_ptr<Robot> r;

    if (type == "worker")
        r = std::make_unique<WorkerRobot>();
    else
        r = std::make_unique<ControllerRobot>();

        r->setPosition(x, y);
        r->setDirection(strToDir(dir));

        if (type == "controller" && req.contains("command")) {
            Command C;
            std::string cmdStr = req["command"];

            if (cmdStr == "rotate_cw")  C.type = CommandType::RotateCW;
            else if (cmdStr == "rotate_ccw") C.type = CommandType::RotateCCW;
            else if (cmdStr == "boost") C.type = CommandType::Boost;
            else                        C.type = CommandType::Broadcast;

            C.dir = strToDir(dir);

            static_cast<ControllerRobot*>(r.get())->setCommand(C);
        }

        eng_.addPlacedRobot(std::move(r));

        auto st = eng_.getStateJson();
        return json{{"status","ok"}, {"state", st["state"]}};
    }
    catch (std::exception &e) {
        return json{{"status","error"}, {"message", e.what()}};
    }
}

    
    // ----------------- RUN STEP -----------------
    if (action == "run_step") {

        // 1) Виконуємо автоматичний хід
        eng_.stepAuto();  

        // 2) Отримуємо оновлений стан
        auto st = eng_.getStateJson();

        // 3) Перевіряємо завершення гри
        bool win  = eng_.isWin();
        bool lose = eng_.isLose();

        return json{
            {"status", "ok"},
            {"state",  st["state"]},
            {"finished", win || lose},
            {"win", win},
            {"lose", lose}
        };
    }

    if (action == "run") {
        eng_.lock();
        return json{{"status","ok"}};
}

    return json{{"status","error"},{"message","unknown action"}};
}