#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <json.hpp>

using json = nlohmann::json;

class WalManager {
public:
    std::vector<json> walBuffer;
    std::unordered_map<std::string, std::string> inMemoryStore;

    void appendWalEntry(std::string cmd, std::string key, std::string value);
    void flushWalToDisk();
    std::unordered_map<std::string, std::string> loadFromWal();
};
