// commandParser.cpp
#include <iostream>
#include <sstream>
#include "commandParser.hpp"

void CommandParser::tokenizerInput(std::string command) {
    std::istringstream iss(command);

    while (iss >> token) {
        tokens.push_back(token);
    }
    action = tokens.front();
    checkExit();
}

void CommandParser::checkExit() {
    if (action == "exit") {
        exit(0);
    }
}

bool CommandParser::validateCommand() {
    if (isValidCommand() && endsWithSemicolon()) {
        if (action == "SET") {
            return validateSetSyntax();
        }
        if (action == "GET") {
            return validateGetSyntax();
        }
        if (action == "DEL") {
            return validateDelSyntax();
        }
    }
    return false;
}

bool CommandParser::isValidCommand() {
    if (validCommands.count(tokens.front()) != 0) {
        return true;
    }
    std::cout << "Parsing Error: Command must be SET, DEL, GET or LIST\n";
    tokens.clear();
    return false;
}

bool CommandParser::endsWithSemicolon() {
    if (!tokens.empty() && tokens.back().back() == ';') {
        return true;
    }
    std::cout << "Parsing Error: Missing ';' at the end\n";
    return false;
}

bool CommandParser::validateSetSyntax() {
    if (tokens.size() == 3) {
        return true;
    }
    std::cout << "Parsing Error: Must have 3 Elements SET and key and value. eg; SET key value;\n";
    return false;
}

bool CommandParser::validateGetSyntax() {
    if (tokens.size() == 2) {
        return true;
    }
    std::cout << "Parsing Error: Must have 2 Elements GET and key to get. eg; GET my_\n";
    return false;
}

bool CommandParser::validateDelSyntax() {
    if (tokens.size() == 2) {
        return true;
    }
    std::cout << "Parsing Error: Must have 2 Elements DEL and key to delete. eg; DEL my_\n";
    return false;
}
