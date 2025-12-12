#include <iostream>
#include <nlohmann/json.hpp>
#include "GameEngine.hpp"
#include "RequestHandler.hpp"

using json = nlohmann::json;

int main() {
    GameEngine engine;
    RequestHandler handler(engine);

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        try {
            json req = json::parse(line);
            json resp = handler.handle(req);
            std::cout << resp.dump() << std::endl;
        } catch (const std::exception& e) {
            json resp = { {"status","error"}, {"message", e.what()} };
            std::cout << resp.dump() << std::endl;
        }
    }
    return 0;
}
