#include "Level.hpp"
#include <algorithm>

Level::Level(int w, int h)
    : width(w), height(h),
      grid(h, std::vector<char>(w, '.')),
      gridCells(h, std::vector<Cell>(w)) {}

void Level::addRobot(std::unique_ptr<Robot> r) {
    robots.push_back(std::move(r));
}

void Level::addWall(int x, int y) {
    if (!isInside(x,y)) return;
    walls.emplace_back(x,y);
    rebuildBackground();
    rebuildCells();
}

void Level::addTarget(int x, int y) {
    if (!isInside(x,y)) return;
    targets.emplace_back(x,y);
    rebuildBackground();
    rebuildCells();
}

void Level::addBox(int x, int y) {
    if (!isInside(x,y)) return;
    boxes.emplace_back(x,y);
    Box b{(int)boxStates.size()+1, x, y, false};
    boxStates.push_back(b);
    rebuildBackground();
    rebuildCells();
}

bool Level::isInside(int x, int y) const {
    return x >= 0 && y >= 0 && x < width && y < height;
}

bool Level::isWall(int x, int y) const {
    return std::find(walls.begin(), walls.end(), std::make_pair(x,y)) != walls.end();
}

bool Level::isTarget(int x, int y) const {
    return std::find(targets.begin(), targets.end(), std::make_pair(x,y)) != targets.end();
}

bool Level::isBox(int x, int y) const {
    return std::find(boxes.begin(), boxes.end(), std::make_pair(x,y)) != boxes.end();
}

void Level::rebuildBackground() {
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            grid[y][x] = '.';

    for (auto& w : walls) grid[w.second][w.first] = 'X';
    for (auto& t : targets) grid[t.second][t.first] = 'T';
    for (auto& b : boxes)  grid[b.second][b.first] = 'b';
}

void Level::rebuildCells() {
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            gridCells[y][x].type = CellType::Empty;

    for (auto& w : walls) gridCells[w.second][w.first].type = CellType::Wall;
    for (auto& t : targets) gridCells[t.second][t.first].type = CellType::Target;
    // коробки можна теж позначати, якщо треба
}

void Level::update() {
    rebuildBackground();
    rebuildCells();

    for (auto& r : robots) {
        auto* st = r->getState();
        if (!st->alive) continue;
        int x = st->x, y = st->y;
        if (!isInside(x,y)) continue;

        if (isWall(x,y)) {
            st->alive = false;
            st->carrying = false;
            st->boxId.reset();
            continue;
        }

        if (st->type == RobotType::Worker && st->carrying && st->boxId) {
            if (isTarget(x,y)) {
                for (auto& b : boxStates) {
                    if (b.id == *st->boxId) b.delivered = true;
                }
                st->carrying = false;
                st->boxId.reset();
            }
        }
    }
}

std::vector<std::vector<char>> Level::getGrid() const {
    return grid;
}

std::vector<std::vector<Cell>>& Level::getGridCells() {
    return gridCells;
}
const std::vector<std::vector<Cell>>& Level::getGridCells() const {
    return gridCells;
}

const std::vector<std::unique_ptr<Robot>>& Level::getRobots() const {
    return robots;
}
std::vector<std::unique_ptr<Robot>>& Level::getRobots() {
    return robots;
}

std::vector<RobotState>& Level::getRobotStates() {
    return robotStates;
}

std::vector<Box>& Level::getBoxes() {
    return boxStates;
}

const std::vector<Box>& Level::getBoxes() const {
    return boxStates;
}


bool Level::isCompleted() const {
    return std::all_of(boxStates.begin(), boxStates.end(),
        [](const Box& b){ return b.delivered; });
}