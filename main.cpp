// AI Tutor https://chatgpt.com/c/6840580c-5b98-800a-8a97-43ebc867fbf5
#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <json.hpp>
#include "walManager.hpp"
#include "commandParser.hpp"


using json = nlohmann::json;


class Database{

    public:
        Database(){
            // TODO: Check why is not loading properly
            // TODO: Add unit tests, starting to get hard to track all things
            keyValueStore = wal.loadFromWal();
        }
                
        void processInput(std::string command){

            KeyValue key_value;
            parser.tokenizerInput(command);
            this->dispatchCommand();
            parser.tokens.clear();
            wal.flushWalToDisk();
            return;
            
        }
    

    private:
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
        
        
        std::string action, key, value;

        bool dispatchCommand(){
            ActionOptions actions;
            if (parser.action == actions.set)
            {
                return this->set();
            }

            if (parser.action == actions.get)
            {
                return this->get();
            }

            if (parser.action == actions.del)
            {
                return this->del();
            }

            if (parser.action == actions.list)
            {
                return this->list();
            }                
            
            return false;
        }

        bool set(){
            if (this->parser.validateCommand())
            {
                key = this->parser.tokens[1];
                value = this->parser.tokens.back();

                keyValueStore[key] = value;
                wal.appendWalEntry("SET", key, value);
                std::cout << "Value inserted!\n";
                return true;
            }
            return false;
        }

        bool get(){
        if (this->parser.validateCommand())
            {
                key = this->parser.tokens.back();
                key.pop_back();
                wal.appendWalEntry("GET", key, keyValueStore[key]);
                std::cout << "\n" << keyValueStore[key] << "\n";
                return true;
            }
            return false;
        }

        bool del(){
        if (parser.validateCommand())
            {
                key = this->parser.tokens.back();
                key.pop_back();
                wal.appendWalEntry("DEL", key, keyValueStore[key]);
                keyValueStore.erase(key);
                std::cout << "Register Erased\n";
                return true;
            }
            return false;
        }

        bool list(){
            wal.appendWalEntry("LIST", "", "");
            std::cout << "\n\nKey \t\tValue\n";
            for (const auto& pair : keyValueStore){
                std::cout << pair.first << "\t\t" << pair.second << "\n";
            };
            return true;

        }

};


int main(int argc, char const *argv[])
{
    std::string command, value, option;
    Database db;

    while (true)
    {
        std::cout << "localdb:: ";
        std::getline(std::cin, command);
        db.processInput(command);
    }
    
    
    return 0;
}
