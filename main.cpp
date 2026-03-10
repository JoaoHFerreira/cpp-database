#include <iostream>
#include <string>
#include "Constants.h"
#include "Table.h"
#include "Statement.h"
#include "MetaCommand.h"

void print_prompt() { std::cout << "db > "; }

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Must supply a database filename." << std::endl;
    exit(EXIT_FAILURE);
  }

  char* filename = argv[1];
  Table* table = new Table(filename);

  std::string input_line;
  while (true) {
    print_prompt();
    if (!std::getline(std::cin, input_line)) {
      std::cout << std::endl;
      break;
    }

    if (input_line.empty()) continue;

    if (input_line[0] == '.') {
      switch (do_meta_command(input_line, table)) {
        case (META_COMMAND_SUCCESS):
          continue;
        case (META_COMMAND_UNRECOGNIZED_COMMAND):
          std::cout << "Unrecognized command '" << input_line << "'" << std::endl;
          continue;
      }
    }

    Statement statement;
    switch (prepare_statement(input_line, &statement)) {
      case (PREPARE_SUCCESS):
        break;
      case (PREPARE_SYNTAX_ERROR):
        std::cout << "Syntax error. Could not parse statement." << std::endl;
        continue;
      case (PREPARE_STRING_TOO_LONG):
        std::cout << "String is too long." << std::endl;
        continue;
      case (PREPARE_UNRECOGNIZED_STATEMENT):
        std::cout << "Unrecognized keyword at start of '" << input_line << "'." << std::endl;
        continue;
    }

    switch (execute_statement(&statement, table)) {
      case (EXECUTE_SUCCESS):
        std::cout << "Executed." << std::endl;
        break;
      case (EXECUTE_DUPLICATE_KEY):
        std::cout << "Error: Duplicate key." << std::endl;
        break;
      case (EXECUTE_TABLE_FULL):
        std::cout << "Error: Table full." << std::endl;
        break;
    }
  }
  return 0;
}
