#pragma once
#include "GameEngine.hpp"
#include "Types.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class RequestHandler {
public:
    explicit RequestHandler(GameEngine& engine);

    json handle(const json& req);

private:
    GameEngine& eng_;

    Command parseCommand(const json& j);
};
