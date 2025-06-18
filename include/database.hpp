// AI Tutor https://chatgpt.com/c/6840580c-5b98-800a-8a97-43ebc867fbf5
#pragma once
#include <string>
#include <unordered_map>
#include "walManager.hpp"
#include "commandParser.hpp"


using json = nlohmann::json;


class Database{
    public:
        Database();
        std::string processInput(std::string command);

    private:
        std::string dbMsg;
        std::mutex dbMutex;
        std::string action, key, value;
        WalManager wal;
        CommandParser parser;
        std::unordered_map<std::string, std::string> keyValueStore;

        struct KeyValue
        {
            std::string key;
            std::string value;
        };

        struct ActionOptions
        {
            std::string set="SET";
            std::string get="GET";
            std::string del="DEL";
            std::string list="LIST;";
        };
        std::string dispatchCommand();
        std::string set();
        std::string get();
        std::string del();
        std::string list();

};
