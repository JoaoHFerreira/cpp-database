#include "Pager.h"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

Pager::Pager(const char* filename) {
  file_descriptor = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
  if (file_descriptor == -1) {
    std::cerr << "Error: Unable to open file" << std::endl;
    exit(EXIT_FAILURE);
  }
  file_length = lseek(file_descriptor, 0, SEEK_END);
  num_pages = file_length / PAGE_SIZE;
  if (file_length % PAGE_SIZE != 0) {
      std::cerr << "Db file is not a whole number of pages. Corrupt file." << std::endl;
      exit(EXIT_FAILURE);
  }

  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    pages[i] = nullptr;
  }
}

Pager::~Pager() {
    // Note: Flush is handled by Table destructor before closing descriptor
}

void* Pager::get_page(uint32_t page_num) {
  if (page_num >= TABLE_MAX_PAGES) {
    std::cerr << "Tried to fetch page number out of bounds. " << page_num << " > " << TABLE_MAX_PAGES << std::endl;
    exit(EXIT_FAILURE);
  }

  if (pages[page_num] == nullptr) {
    // Cache miss. Allocate memory and load from file.
    void* page = malloc(PAGE_SIZE);
    memset(page, 0, PAGE_SIZE);
    uint32_t num_pages_in_file = file_length / PAGE_SIZE;

    if (page_num < num_pages_in_file) {
      lseek(file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
      ssize_t bytes_read = read(file_descriptor, page, PAGE_SIZE);
      if (bytes_read == -1) {
        std::cerr << "Error reading file: " << errno << std::endl;
        exit(EXIT_FAILURE);
      }
    }

    pages[page_num] = page;
    if (page_num >= num_pages) {
        num_pages = page_num + 1;
    }
  }

  return pages[page_num];
}

uint32_t Pager::get_unused_page_num() { return num_pages; }

void Pager::flush(uint32_t page_num) {
  if (pages[page_num] == nullptr) {
    return;
  }

  lseek(file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
  ssize_t bytes_written = write(file_descriptor, pages[page_num], PAGE_SIZE);

  if (bytes_written == -1) {
    std::cerr << "Error writing file: " << errno << std::endl;
    exit(EXIT_FAILURE);
  }
}
