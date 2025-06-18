#include "database.hpp"
#include <iostream>

Database::Database() {
    keyValueStore = wal.loadFromWal();
}

std::string Database::processInput(std::string command) {
    parser.tokenizerInput(command);
    dbMsg = dispatchCommand();
    parser.tokens.clear();
    wal.flushWalToDisk();
    return dbMsg;
}

std::string Database::dispatchCommand() {
    ActionOptions actions;
    dbMsg = parser.validateCommand();
    if (dbMsg == "")
    {
        if (parser.action == actions.set) {
            dbMsg = set();
            return dbMsg;
        }
        if (parser.action == actions.get) {
            dbMsg = get();
            return dbMsg;
        }
        if (parser.action == actions.del) {
            dbMsg = del();
            return dbMsg;
        }
        if (parser.action == actions.list) {
            dbMsg = list();
            return dbMsg;
        } 
    }
    
    return dbMsg;
}

std::string Database::set() {
    if (parser.validateSetSyntax()) {
        std::lock_guard<std::mutex> lock(dbMutex);
        key = parser.tokens[1];
        value = parser.tokens.back();
        keyValueStore[key] = value;
        wal.appendWalEntry("SET", key, value);
        return "Value inserted!\n";
    }
    return "Parsing Error: Must have 3 Elements SET and key and value. eg; SET key value;\n";
}

std::string Database::get() {
    if (parser.validateGetSyntax()) {
        std::lock_guard<std::mutex> lock(dbMutex);
        key = parser.tokens.back();
        key.pop_back();
        wal.appendWalEntry("GET", key, keyValueStore[key]);
        return "\n" + keyValueStore[key] + "\n";
    }
    return "Parsing Error: Must have 2 Elements GET and key to get. eg; GET my_\n";
}

std::string Database::del() {
    if (parser.validateDelSyntax()) {
        std::lock_guard<std::mutex> lock(dbMutex);
        key = parser.tokens.back();
        key.pop_back();
        wal.appendWalEntry("DEL", key, keyValueStore[key]);
        keyValueStore.erase(key);
        return "Register Erased\n";
    }
    return "Parsing Error: Must have 2 Elements DEL and key to delete. eg; DEL my_\n";
}

std::string Database::list() {
    std::string result;
    std::lock_guard<std::mutex> lock(dbMutex);
    
    wal.appendWalEntry("LIST", "", "");

    result = "";
    for (const auto& pair : keyValueStore) {
        result += pair.first + "\t\t" + pair.second + "\n";
    }
    
    if (result == "")
    {
        result += "No Registers\n";
        return result;
    }
    
    result += std::string("Finish Registers LIST!") + "\n";
    return result;
}
