#include "RequestHandler.hpp"
#include "LevelLoader.hpp"   // тепер підключаємо хедер, а не cpp
#include <stdexcept>

using json = nlohmann::json;

static Direction parseDir(const std::string& s) {
    if (s == "up") return Direction::Up;
    if (s == "down") return Direction::Down;
    if (s == "left") return Direction::Left;
    return Direction::Right;
}

static CommandType parseCmdType(const std::string& s) {
    if (s == "move") return CommandType::Move;
    if (s == "pick") return CommandType::Pick;
    if (s == "drop") return CommandType::Drop;
    if (s == "give") return CommandType::Give;
    return CommandType::Broadcast;
}

Command RequestHandler::parseCommand(const json& j) {
    Command c;
    c.robotId = j.at("robot_id").get<int>();
    c.type = parseCmdType(j.at("cmd").get<std::string>());
    if (j.contains("dir")) c.dir = parseDir(j.at("dir").get<std::string>());
    return c;
}

json RequestHandler::handle(const json& req) {
    const auto action = req.value("action", "");

    if (action == "load_level") {
        const auto path = req.at("path").get<std::string>();
        Level lvl = loadLevelFromFile(path);
        eng_.loadLevel(std::move(lvl));
        return json{{"status","ok"},{"state", eng_.getStateJson()}};
    }

    if (action == "status") {
        return json{{"status","ok"},{"state", eng_.getStateJson()}};
    }

    if (action == "step") {
        std::vector<Command> cmds;
        if (req.contains("commands")) {
            for (const auto& jc : req.at("commands")) {
                cmds.push_back(parseCommand(jc));
            }
        }
        eng_.applyCommands(cmds);
        return json{{"status","ok"},{"state", eng_.getStateJson()}};
    }

    return json{{"status","error"},{"message","unknown action"}};
}