// AI Tutor https://chatgpt.com/c/6840580c-5b98-800a-8a97-43ebc867fbf5
#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include "json.hpp"

using json = nlohmann::json;

class Database{

    public:
        void process_command(std::string command){

            KeyValue key_value;
            this->feed_internal_array(command);

            if (this->check_command()){
                this->execute();
                this->splited_command.clear();
                this->incremental_index++;
                this->save_wal();
                return;
            }
            this->splited_command.clear();
            return;
        }
    

    private:
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
        
        
        const std::unordered_set<std::string> valid_commands = {"SET", "DEL", "GET", "LIST;"};
        std::vector<std::string> splited_command;
        std::unordered_map<std::string, std::string> in_memory;
        std::unordered_map<int, std::unordered_map<std::string, std::string>> wal_file;
        std::string action, key, value;
        int incremental_index=0;

        bool execute(){
            if (this->splited_command.back().back() != ';')
            {
                std::cout << "Parsing Error: Missing ';' at the end\n";
                return false;
            }

            ActionOptions actions;
            action = this->splited_command.front();

            if (action == actions.set)
            {
                return this->set();
            }

            if (action == actions.get)
            {
                return this->get();
            }

            if (action == actions.del)
            {
                return this->del();
            }

            if (action == actions.list)
            {
                return this->list();
            }
            return false;
        }

        bool set(){
        if (this->splited_command.size() != 3)
            {
                std::cout << "Parsing Error: Must have 3 Elements SET and key and value. eg; SET key value;\n";
                return false;
            }
        key = this->splited_command[1];
        value = splited_command.back();

        in_memory[key] = value;

        this->wal_file[this->incremental_index]["cmd"] = "SET";
        this->wal_file[this->incremental_index]["key"] = key;
        this->wal_file[this->incremental_index]["value"] = value;

        std::cout << "Value inserted!\n";
        return true;
        }

        bool get(){
        if (this->splited_command.size() != 2)
            {
                std::cout << "Parsing Error: Must have 2 Elements GET and key to get. eg; GET my_\n";
                return false;
            }
        key = this->splited_command.back();

        key.pop_back();

        this->wal_file[this->incremental_index]["cmd"] = "GET";
        this->wal_file[this->incremental_index]["key"] = key;
        this->wal_file[this->incremental_index]["value"] = in_memory[key];

        std::cout << "\n" << in_memory[key] << "\n";
        return true;
        }

        bool del(){
        if (this->splited_command.size() != 2)
            {
                std::cout << "Parsing Error: Must have 2 Elements DEL and key to delete. eg; DEL my_\n";
                return false;
            }
        
        key = this->splited_command.back();
        key.pop_back();
        
        this->wal_file[this->incremental_index]["cmd"] = "DEL";
        this->wal_file[this->incremental_index]["key"] = key;
        this->wal_file[this->incremental_index]["value"] = in_memory[key];


        in_memory.erase(key);
        
        std::cout << "Register Erased\n";
        return true;
        }

        bool list(){
            this->wal_file[this->incremental_index]["cmd"] = "LIST";
            this->wal_file[this->incremental_index]["key"] = "";
            this->wal_file[this->incremental_index]["value"] = "";

            std::cout << "\n\nKey \t\tValue\n";
            for (const auto& pair : in_memory){
                std::cout << pair.first << "\t\t" << pair.second << "\n";
            };
            return true;

        }

        void feed_internal_array(std::string command){
            std::istringstream iss(command);
            std::string word;

            while (iss >> word)
            {
                splited_command.push_back(word);
            }            
            
        }

        bool check_command(){
            if (valid_commands.count(this->splited_command.front()) == 0) {
                std::cout << "Parsing Error: Command must be SET, DEL, GET or LIST\n";
                return false;
            }
            return true;

        }

        void save_wal() {
            std::ofstream file("wal.jsonl", std::ios::app);
            for (auto& [index, entry] : this->wal_file) {
                json command = { index, entry };
                file << command.dump() << "\n";
            }
            file.close();
            this->wal_file.clear();
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
        db.process_command(command);
    }
    
    
    return 0;
}
