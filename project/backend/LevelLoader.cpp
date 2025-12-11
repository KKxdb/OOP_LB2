#include "LevelLoader.hpp"
#include "WorkerRobot.hpp"
#include "ControllerRobot.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

Level loadLevelFromFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open())
        throw std::runtime_error("Cannot open level file: " + path);

    json j;
    f >> j;

    int w = j.value("width", 10);
    int h = j.value("height", 10);

    Level lvl(w, h);

    auto& wv = j.at("world");

    int nextRobotId = 1;

    // --- WALLS ---
    if (wv.contains("walls"))
        for (auto& p : wv["walls"])
            lvl.addWall(p[0], p[1]);

    // --- TARGETS ---
    if (wv.contains("targets"))
        for (auto& p : wv["targets"])
            lvl.addTarget(p[0], p[1]);

    // --- BOXES ---
    if (wv.contains("boxes"))
        for (auto& p : wv["boxes"])
            lvl.addBox(p[0], p[1]);

    // --- ROBOTS ---
    if (wv.contains("robots")) {
        for (auto& r : wv["robots"]) {
            std::string t = r["type"];
            int x = r["x"];
            int y = r["y"];

            RobotType rt = (t=="worker") ? RobotType::Worker : RobotType::Controller;

            RobotState rs{nextRobotId++, rt, x, y};
            lvl.getRobotStates().push_back(rs);

            if (rt == RobotType::Worker)
                lvl.addRobot(std::make_unique<WorkerRobot>(&lvl.getRobotStates().back()));
            else
                lvl.addRobot(std::make_unique<ControllerRobot>(&lvl.getRobotStates().back()));
        }
    }

    return lvl;
}
