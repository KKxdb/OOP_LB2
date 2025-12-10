#include "LevelLoader.hpp"
#include "WorkerRobot.hpp"
#include "ControllerRobot.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

static CellType parseCellType(const std::string& s) {
    if (s == "wall") return CellType::Wall;
    if (s == "target") return CellType::Target;
    return CellType::Empty;
}

Level loadLevelFromFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) throw std::runtime_error("Не вдалося відкрити файл: " + path);

    json j; 
    f >> j;

    int w = j.value("width", 10);
    int h = j.value("height", 10);
    Level lvl(w, h);

    auto gridJson = j.at("grid");

    int nextRobotId = 1;
    int nextBoxId = 1;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            std::string cell = gridJson.at(y).at(x).get<std::string>();

            if (cell == "worker") {
                RobotState rs{nextRobotId++, RobotType::Worker, x, y};
                lvl.getRobotStates().push_back(rs);
                lvl.addRobot(std::make_unique<WorkerRobot>(&lvl.getRobotStates().back()));
            } 
            else if (cell == "controller") {
                RobotState rs{nextRobotId++, RobotType::Controller, x, y};
                lvl.getRobotStates().push_back(rs);
                lvl.addRobot(std::make_unique<ControllerRobot>(&lvl.getRobotStates().back()));
            } 
            else if (cell == "box") {
                Box b{nextBoxId++, x, y, false};
                lvl.getBoxes().push_back(b);
                lvl.addBox(x, y);
            } 
            else if (cell == "wall") {
                lvl.addWall(x, y);
            } 
            else if (cell == "target") {
                lvl.addTarget(x, y);
            }
            // "empty" нічого не додає
        }
    }

    return lvl;
}