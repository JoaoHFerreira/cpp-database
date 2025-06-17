#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

class CommandParser{
    public:
        std::vector<std::string> tokens;
        std::string token, action;
        const std::unordered_set<std::string> validCommands = {"SET", "DEL", "GET", "LIST;"};

        void tokenizerInput(std::string command);
        void checkExit();
        bool validateCommand();
        bool isValidCommand();
        bool endsWithSemicolon();
        bool validateSetSyntax();        
        bool validateGetSyntax();
        bool validateDelSyntax();
};