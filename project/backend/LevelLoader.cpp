#include "LevelLoader.hpp"
#include "Level.hpp"
#include "WorkerRobot.hpp"
#include "ControllerRobot.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <codecvt>

using json = nlohmann::json;

std::optional<Level> LevelLoader::loadFromJson(const std::string& path) {
    #ifdef _WIN32
        // конвертуємо UTF-8 → UTF-16 (Windows wide path)
        std::wstring ws = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.from_bytes(path);

        std::ifstream f(ws, std::ios::binary);
    #else
        std::ifstream f(path, std::ios::binary);
    #endif

    if (!f.is_open()) {
        std::cerr << "Cannot open file: " << path << std::endl;
        return std::nullopt;
    }

    json j;
    try { f >> j; }
    catch (...) { return std::nullopt; }

    int W = j.value("width", 10);
    int H = j.value("height", 10);
    Level lvl(W, H);

    if (j.contains("world")) {
        auto &w = j["world"];
        if (w.contains("walls")) for (auto &p : w["walls"]) lvl.addWall(p[0], p[1]);
        if (w.contains("targets")) for (auto &p : w["targets"]) lvl.addTarget(p[0], p[1]);
        if (w.contains("boxes")) for (auto &p : w["boxes"]) lvl.addBox(p[0], p[1]);
        if (w.contains("robots")) {
            for (auto &r : w["robots"]) {
                std::string type = r.value("type", "worker");
                int x = r.value("x", 0);
                int y = r.value("y", 0);
                std::unique_ptr<Robot> rp;
                if (type == "worker") rp = std::make_unique<WorkerRobot>();
                else rp = std::make_unique<ControllerRobot>();
                rp->setPosition(x,y);
                lvl.addRobot(std::move(rp));
            }
        }
    } else {
        // старий простий формат (за потреби)
        if (j.contains("walls")) for (auto &p : j["walls"]) lvl.addWall(p[0], p[1]);
        if (j.contains("targets")) for (auto &p : j["targets"]) lvl.addTarget(p[0], p[1]);
        if (j.contains("boxes")) for (auto &p : j["boxes"]) lvl.addBox(p[0], p[1]);
        if (j.contains("robots")) {
            for (auto &r : j["robots"]) {
                std::string type = r.value("type", "worker");
                int x = r.value("x", 0);
                int y = r.value("y", 0);
                std::unique_ptr<Robot> rp;
                if (type == "worker") rp = std::make_unique<WorkerRobot>();
                else rp = std::make_unique<ControllerRobot>();
                rp->setPosition(x,y);
                lvl.addRobot(std::move(rp));
            }
        }
    }

    return lvl;
}