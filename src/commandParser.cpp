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

std::string CommandParser::validateCommand() {
    if (isValidCommand() && endsWithSemicolon()) {
        return "";
    }
    return "Parsing Error: Command must be SET, DEL, GET or LIST\n"
       "Parsing Error: Missing ';' at the end\n";
}

bool CommandParser::isValidCommand() {
    if (validCommands.count(tokens.front()) != 0) {
        return true;
    }
    tokens.clear();
    return false;
}

bool CommandParser::endsWithSemicolon() {
    if (!tokens.empty() && tokens.back().back() == ';') {
        return true;
    }
    return false;
}

bool CommandParser::validateSetSyntax() {
    if (tokens.size() == 3) {
        return true;
    }
    return false;
}

bool CommandParser::validateGetSyntax() {
    if (tokens.size() == 2) {
        return true;
    }
    return false;
}

bool CommandParser::validateDelSyntax() {
    if (tokens.size() == 2) {
        return true;
    }
    return false;
}
