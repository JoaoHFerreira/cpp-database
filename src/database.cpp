#include "database.hpp"
#include <iostream>

Database::Database() {
    keyValueStore = wal.loadFromWal();
}

void Database::processInput(std::string command) {
    parser.tokenizerInput(command);
    dispatchCommand();
    parser.tokens.clear();
    wal.flushWalToDisk();
}

bool Database::dispatchCommand() {
    ActionOptions actions;
    if (parser.action == actions.set) {
        return set();
    }
    if (parser.action == actions.get) {
        return get();
    }
    if (parser.action == actions.del) {
        return del();
    }
    if (parser.action == actions.list) {
        return list();
    }
    return false;
}

bool Database::set() {
    if (parser.validateCommand()) {
        key = parser.tokens[1];
        value = parser.tokens.back();
        keyValueStore[key] = value;
        wal.appendWalEntry("SET", key, value);
        std::cout << "Value inserted!\n";
        return true;
    }
    return false;
}

bool Database::get() {
    if (parser.validateCommand()) {
        key = parser.tokens.back();
        key.pop_back();  // Remove trailing semicolon
        wal.appendWalEntry("GET", key, keyValueStore[key]);
        std::cout << "\n" << keyValueStore[key] << "\n";
        return true;
    }
    return false;
}

bool Database::del() {
    if (parser.validateCommand()) {
        key = parser.tokens.back();
        key.pop_back();  // Remove trailing semicolon
        wal.appendWalEntry("DEL", key, keyValueStore[key]);
        keyValueStore.erase(key);
        std::cout << "Register Erased\n";
        return true;
    }
    return false;
}

bool Database::list() {
    wal.appendWalEntry("LIST", "", "");
    std::cout << "\n\nKey \t\tValue\n";
    for (const auto& pair : keyValueStore) {
        std::cout << pair.first << "\t\t" << pair.second << "\n";
    }
    return true;
}
