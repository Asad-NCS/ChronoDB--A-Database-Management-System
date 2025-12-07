#include <iostream>
#include <string>
#include "../storage/storage.h"
#include "../query/parser.h"
#include "../graph/graph.h"
#include "../utils/helpers.h"

using namespace ChronoDB;

int main() {
    StorageEngine storage;
    GraphEngine graph;
    Parser parser(storage, graph);

    std::string inputLine;
    std::string commandBuffer;

    std::cout << "=== ChronoDB SQL CLI ===" << std::endl;
    std::cout << "Type 'EXIT' to quit." << std::endl;

    while (true) {
        if (commandBuffer.empty())
            std::cout << "ChronoDB> ";
        else
            std::cout << "....> ";

        std::getline(std::cin, inputLine);

        if (inputLine.empty()) continue;

        // Check for EXIT (case-insensitive)
        std::string inputUpper = Helper::toUpper(inputLine);
        if (inputUpper == "EXIT") break;

        commandBuffer += inputLine + " ";

        // If command ends with ';', execute it
        if (inputLine.back() == ';') {
            // Remove trailing "; " (semicolon + space)
            commandBuffer.pop_back();  // Remove space
            commandBuffer.pop_back();  // Remove semicolon
            parser.parseAndExecute(commandBuffer);
            commandBuffer.clear();
        }
    }

    return 0;
}
