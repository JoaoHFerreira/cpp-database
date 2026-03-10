#ifndef META_COMMAND_H
#define META_COMMAND_H

#include "Constants.h"
#include "Table.h"
#include <string>

MetaCommandResult do_meta_command(std::string input, Table* table);

#endif
