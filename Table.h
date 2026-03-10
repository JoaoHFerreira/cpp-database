#ifndef TABLE_H
#define TABLE_H

#include "Pager.h"
#include <cstdint>

/**
 * Table represents a collection of records.
 * It manages the root of the B-Tree and the Pager.
 */
struct Table {
  uint32_t root_page_num;
  Pager* pager;

  Table(const char* filename);
  ~Table();
};

#endif
