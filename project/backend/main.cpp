#include <iostream>
#include <string>
#include "GameEngine.hpp"

int main() {
    GameEngine engine;
    std::string input;

    while (std::getline(std::cin, input)) {
        std::string output = engine.processCommand(input);
        std::cout << output << std::endl;
    }

    return 0;
}
