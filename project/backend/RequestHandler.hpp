#pragma once
#include "GameEngine.hpp"
#include "Types.hpp"
#include <nlohmann/json.hpp>
#include <string>

class RequestHandler {
public:
    explicit RequestHandler(GameEngine& engine) : eng_(engine) {}

    // головний метод: приймає JSON-запит, повертає JSON-відповідь
    nlohmann::json handle(const nlohmann::json& req);

private:
    GameEngine& eng_;

    Command parseCommand(const nlohmann::json& j);
};