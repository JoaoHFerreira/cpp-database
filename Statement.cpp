#include "Statement.h"
#include "Node.h"
#include "Cursor.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>

/**
 * SQL Statement Handling
 * This module parses input strings into Statement objects and executes them.
 */

// Helper function to convert string to lowercase for case-insensitive parsing
std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

void internal_node_insert(Table* table, uint32_t parent_page_num, uint32_t child_page_num) {
    void* parent = table->pager->get_page(parent_page_num);
    void* child = table->pager->get_page(child_page_num);
    uint32_t child_max_key = get_node_max_key(child);
    uint32_t index = 0;
    uint32_t num_keys = *internal_node_num_keys(parent);

    while (index < num_keys && *internal_node_key(parent, index) < child_max_key) {
        index++;
    }

    if (num_keys >= 3) {
        std::cerr << "Need to implement splitting internal node" << std::endl;
        exit(EXIT_FAILURE);
    }

    uint32_t right_child_page_num = *internal_node_right_child(parent);

    if (child_max_key > get_node_max_key(table->pager->get_page(right_child_page_num))) {
        *internal_node_child(parent, num_keys) = right_child_page_num;
        *internal_node_key(parent, num_keys) = get_node_max_key(table->pager->get_page(right_child_page_num));
        *internal_node_right_child(parent) = child_page_num;
    } else {
        for (uint32_t i = num_keys; i > index; i--) {
            memcpy(internal_node_cell(parent, i), internal_node_cell(parent, i - 1), INTERNAL_NODE_CELL_SIZE);
        }
        *internal_node_child(parent, index) = child_page_num;
        *internal_node_key(parent, index) = child_max_key;
    }
    *internal_node_num_keys(parent) = num_keys + 1;
}

void leaf_node_split_and_insert(Cursor* cursor, uint32_t key, Row* value) {
    void* old_node = cursor->table->pager->get_page(cursor->page_num);
    uint32_t old_max_key = get_node_max_key(old_node);
    uint32_t new_page_num = cursor->table->pager->get_unused_page_num();
    void* new_node = cursor->table->pager->get_page(new_page_num);
    initialize_leaf_node(new_node);
    *node_parent(new_node) = *node_parent(old_node);
    *leaf_node_next_leaf(new_node) = *leaf_node_next_leaf(old_node);
    *leaf_node_next_leaf(old_node) = new_page_num;

    for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--) {
        void* destination_node = (i >= (int32_t)LEAF_NODE_LEFT_SPLIT_COUNT) ? new_node : old_node;
        uint32_t index_within_node = (i >= (int32_t)LEAF_NODE_LEFT_SPLIT_COUNT) ? i - LEAF_NODE_LEFT_SPLIT_COUNT : i;
        void* destination = leaf_node_cell(destination_node, index_within_node);

        if (i == (int32_t)cursor->cell_num) {
            serialize_row(value, leaf_node_value(destination_node, index_within_node));
            *leaf_node_key(destination_node, index_within_node) = key;
        } else if (i > (int32_t)cursor->cell_num) {
            memcpy(destination, leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
        } else {
            memcpy(destination, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
        }
    }

    *(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
    *(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

    if (is_node_root(old_node)) {
        create_new_root(cursor->table, new_page_num);
    } else {
        uint32_t parent_page_num = *node_parent(old_node);
        void* parent = cursor->table->pager->get_page(parent_page_num);
        update_internal_node_key(parent, old_max_key, get_node_max_key(old_node));
        internal_node_insert(cursor->table, parent_page_num, new_page_num);
    }
}

void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value) {
    void* node = cursor->table->pager->get_page(cursor->page_num);

    uint32_t num_cells = *leaf_node_num_cells(node);
    if (num_cells >= LEAF_NODE_MAX_CELLS) {
        leaf_node_split_and_insert(cursor, key, value);
        return;
    }

    if (cursor->cell_num < num_cells) {
        for (uint32_t i = num_cells; i > cursor->cell_num; i--) {
            memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1), LEAF_NODE_CELL_SIZE);
        }
    }

    *(leaf_node_num_cells(node)) += 1;
    *(leaf_node_key(node, cursor->cell_num)) = key;
    serialize_row(value, leaf_node_value(node, cursor->cell_num));
}

PrepareResult prepare_statement(std::string input, Statement* statement) {
  std::string lower_input = to_lower(input);

  if (lower_input.substr(0, 6) == "insert") {
    statement->type = STATEMENT_INSERT;
    std::stringstream ss(input); // Use original for case-sensitive data if any (though username/email usually are)
    std::string keyword, username, email;
    uint32_t id;
    ss >> keyword;
    if (!(ss >> id >> username >> email)) {
      return PREPARE_SYNTAX_ERROR;
    }
    if (username.length() >= COLUMN_USERNAME_SIZE || email.length() >= COLUMN_EMAIL_SIZE) {
        return PREPARE_STRING_TOO_LONG;
    }
    statement->row_to_insert.id = id;
    strcpy(statement->row_to_insert.username, username.c_str());
    strcpy(statement->row_to_insert.email, email.c_str());
    return PREPARE_SUCCESS;
  }

  if (lower_input.substr(0, 6) == "select") {
    statement->type = STATEMENT_SELECT;
    statement->has_where = false;

    // Simple ANSI SQL parser: SELECT * FROM users [WHERE id = X]
    std::stringstream ss(lower_input);
    std::string word;
    ss >> word; // select

    if (!(ss >> word) || word != "*") return PREPARE_SYNTAX_ERROR;
    if (!(ss >> word) || word != "from") return PREPARE_SYNTAX_ERROR;
    if (!(ss >> word)) return PREPARE_SYNTAX_ERROR; // table name (ignored for now)

    if (ss >> word) {
        if (word == "where") {
            if (!(ss >> word) || word != "id") return PREPARE_SYNTAX_ERROR;
            if (!(ss >> word) || word != "=") return PREPARE_SYNTAX_ERROR;
            if (!(ss >> statement->where_id)) return PREPARE_SYNTAX_ERROR;
            statement->has_where = true;
        } else {
            return PREPARE_SYNTAX_ERROR;
        }
    }

    return PREPARE_SUCCESS;
  }

  return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult execute_insert(Statement* statement, Table* table) {
  Row* row_to_insert = &(statement->row_to_insert);
  uint32_t key_to_insert = row_to_insert->id;
  Cursor cursor(table, key_to_insert);

  void* node = table->pager->get_page(cursor.page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);
  if (cursor.cell_num < num_cells) {
      uint32_t key_at_index = *leaf_node_key(node, cursor.cell_num);
      if (key_at_index == key_to_insert) {
          return EXECUTE_DUPLICATE_KEY;
      }
  }

  leaf_node_insert(&cursor, row_to_insert->id, row_to_insert);
  return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement* statement, Table* table) {
  Row row;
  if (statement->has_where) {
      Cursor cursor(table, statement->where_id);
      void* node = table->pager->get_page(cursor.page_num);
      uint32_t num_cells = *leaf_node_num_cells(node);
      if (cursor.cell_num < num_cells) {
          uint32_t key_at_index = *leaf_node_key(node, cursor.cell_num);
          if (key_at_index == statement->where_id) {
              deserialize_row(cursor.value(), &row);
              print_row(&row);
          }
      }
  } else {
      Cursor cursor(table, true);
      while (!(cursor.end_of_table)) {
          deserialize_row(cursor.value(), &row);
          print_row(&row);
          cursor.advance();
      }
  }
  return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement* statement, Table* table) {
  switch (statement->type) {
    case (STATEMENT_INSERT):
      return execute_insert(statement, table);
    case (STATEMENT_SELECT):
      return execute_select(statement, table);
  }
  return EXECUTE_SUCCESS;
}
