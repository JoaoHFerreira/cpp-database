#ifndef CURSOR_H
#define CURSOR_H

#include "Table.h"
#include <cstdint>

/**
 * Cursor represents a position in the table.
 * It simplifies traversing the B-Tree.
 */
struct Cursor {
  Table* table;
  uint32_t page_num;
  uint32_t cell_num;
  bool end_of_table;

  Cursor(Table* table, uint32_t key);
  Cursor(Table* table, bool start);

  void* value();
  void advance();

private:
  void leaf_node_find(Table* table, uint32_t page_num, uint32_t key);
  uint32_t internal_node_find_child(void* node, uint32_t key);
  void tree_find(Table* table, uint32_t page_num, uint32_t key);
};

#endif
