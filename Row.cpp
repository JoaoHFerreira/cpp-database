#include "Row.h"
#include <cstring>
#include <iostream>

/**
 * serialize_row copies the fields of a Row struct into a compact byte buffer.
 * C++ specific: We use memcpy to perform a bitwise copy. char* casting
 * allows us to treat the destination memory as a byte array for offset calculation.
 */
void serialize_row(Row* source, void* destination) {
  memcpy((char*)destination + ID_OFFSET, &(source->id), ID_SIZE);
  memcpy((char*)destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
  memcpy((char*)destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

/**
 * deserialize_row reconstructs a Row struct from a byte buffer.
 * This is the reverse of serialize_row.
 */
void deserialize_row(void* source, Row* destination) {
  memcpy(&(destination->id), (char*)source + ID_OFFSET, ID_SIZE);
  memcpy(&(destination->username), (char*)source + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy(&(destination->email), (char*)source + EMAIL_OFFSET, EMAIL_SIZE);
}

void print_row(Row* row) {
  std::cout << "(" << row->id << ", " << row->username << ", " << row->email << ")" << std::endl;
}
