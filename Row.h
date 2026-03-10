#ifndef ROW_H
#define ROW_H

#include "Constants.h"
#include <cstdint>

/**
 * Row represents a single record in the database.
 * We use fixed-size character arrays to ensure each row has a predictable size.
 */
struct Row {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE];
  char email[COLUMN_EMAIL_SIZE];
};

/**
 * ROW_SIZE is the total size of a Row object when serialized.
 * We calculate this manually to avoid compiler padding in the struct.
 */
const uint32_t ID_SIZE = sizeof(uint32_t);
const uint32_t USERNAME_SIZE = COLUMN_USERNAME_SIZE;
const uint32_t EMAIL_SIZE = COLUMN_EMAIL_SIZE;
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

void serialize_row(Row* source, void* destination);
void deserialize_row(void* source, Row* destination);
void print_row(Row* row);

#endif
