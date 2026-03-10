#include "Table.h"
#include "Node.h"
#include <unistd.h>
#include <iostream>

Table::Table(const char* filename) {
  pager = new Pager(filename);
  root_page_num = 0;
  if (pager->num_pages == 0) {
    // New database file. Initialize page 0 as leaf node.
    void* root_node = pager->get_page(0);
    initialize_leaf_node(root_node);
    set_node_root(root_node, true);
  }
}

Table::~Table() {
  for (uint32_t i = 0; i < pager->num_pages; i++) {
    if (pager->pages[i] == nullptr) {
      continue;
    }
    pager->flush(i);
    free(pager->pages[i]);
    pager->pages[i] = nullptr;
  }

  int result = close(pager->file_descriptor);
  if (result == -1) {
    std::cerr << "Error closing db file." << std::endl;
    exit(EXIT_FAILURE);
  }

  delete pager;
}
