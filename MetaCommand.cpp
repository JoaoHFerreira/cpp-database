#include "MetaCommand.h"
#include "Row.h"
#include "Node.h"
#include <iostream>

MetaCommandResult do_meta_command(std::string input, Table* table) {
  if (input == ".exit") {
    delete table;
    exit(EXIT_SUCCESS);
  } else if (input == ".constants") {
      std::cout << "Constants:" << std::endl;
      std::cout << "ROW_SIZE: " << ROW_SIZE << std::endl;
      std::cout << "LEAF_NODE_MAX_CELLS: " << LEAF_NODE_MAX_CELLS << std::endl;
      return META_COMMAND_SUCCESS;
  } else {
    return META_COMMAND_UNRECOGNIZED_COMMAND;
  }
}
