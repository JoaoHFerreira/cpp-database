#ifndef STATEMENT_H
#define STATEMENT_H

#include "Constants.h"
#include "Row.h"
#include "Table.h"
#include <string>

enum StatementType { STATEMENT_INSERT, STATEMENT_SELECT };

struct Statement {
  StatementType type;
  Row row_to_insert;
  bool has_where;
  uint32_t where_id;
};

PrepareResult prepare_statement(std::string input, Statement* statement);
ExecuteResult execute_statement(Statement* statement, Table* table);

#endif
