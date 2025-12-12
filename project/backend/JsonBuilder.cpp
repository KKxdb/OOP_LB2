#include "JsonBuilder.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

json buildLevelJson(const Level& lvl)
{
    json j;
    // grid (символи)
    j["grid"] = json::array();
    auto grid = lvl.getGrid();
    for (int y = 0; y < lvl.getHeight(); ++y) {
        json row = json::array();
        for (int x = 0; x < lvl.getWidth(); ++x) {
            row.push_back(static_cast<int>(grid[y][x])); // char -> int (46/88/..)
        }
        j["grid"].push_back(row);
    }

    // state
    json st;
    st["width"] = lvl.getWidth();
    st["height"] = lvl.getHeight();

    // walls
    st["walls"] = json::array();
    for (int y=0; y<lvl.getHeight(); ++y)
        for (int x=0; x<lvl.getWidth(); ++x)
            if (lvl.isWall(x,y)) st["walls"].push_back({{"x", x}, {"y", y}});

    // targets
    st["targets"] = json::array();
    for (auto &t : lvl.getTargets()) st["targets"].push_back({{"x", t.first}, {"y", t.second}});

    // boxes
    st["boxes"] = json::array();
    for (auto& b : lvl.getBoxes()) {
        st["boxes"].push_back({
            {"id", b.id},
            {"x", b.x},
            {"y", b.y},
            {"delivered", b.delivered}
        });
    }

    // robots (з state-у)
    st["robots"] = json::array();
    for (auto& rptr : lvl.getRobots()) {
        auto* s = rptr->getState();
        if (!s) continue;
        st["robots"].push_back({
            {"id", s->id},
            {"type", s->type == RobotType::Worker ? "worker" : "controller"},
            {"x", s->x},
            {"y", s->y},
            {"alive", s->alive},
            {"carrying", s->carrying},
            {"box_id", s->boxId ? *s->boxId : -1}
        });
    }

    j["state"] = st;
    return j;
}
