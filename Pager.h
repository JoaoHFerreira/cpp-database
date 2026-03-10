#ifndef PAGER_H
#define PAGER_H

#include <cstdint>
#include "Constants.h"

/**
 * Pager handles reading and writing pages of data to/from disk.
 * It also maintains an in-memory cache of pages.
 */
struct Pager {
  int file_descriptor;
  uint32_t file_length;
  uint32_t num_pages;
  void* pages[TABLE_MAX_PAGES];

  Pager(const char* filename);
  ~Pager();
  void* get_page(uint32_t page_num);
  uint32_t get_unused_page_num();
  void flush(uint32_t page_num);
};

#endif
