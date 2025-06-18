#include "walManager.hpp"
#include <fstream>

void WalManager::appendWalEntry(std::string cmd, std::string key, std::string value) {
    std::lock_guard<std::mutex> lock(walMutex);
    walBuffer.push_back({
        {"cmd", cmd},
        {"key", key},
        {"value", value}
    });
}

void WalManager::flushWalToDisk() {
    std::lock_guard<std::mutex> lock(walMutex);
    std::ofstream file("wal.jsonl", std::ios::app);
    for (const auto& entry : walBuffer) {
        file << entry.dump() << "\n";
    }
    file.close();
    walBuffer.clear();
}

std::unordered_map<std::string, std::string> WalManager::loadFromWal() {
    std::lock_guard<std::mutex> lock(walMutex);
    std::ifstream file("wal.jsonl");
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        json entry = json::parse(line);

        if (entry["cmd"] == "SET") {
            inMemoryStore[entry["key"]] = entry["value"];
        } else if (entry["cmd"] == "DEL") {
            inMemoryStore.erase(entry["key"]);
        }
    }

    file.close();
    return inMemoryStore;
}
