#include "Node.h"
#include <cstring>
#include <iostream>

/**
 * B-Tree Node Implementation
 * This module provides functions to interact with the raw byte buffers
 * that represent B-Tree nodes.
 */

/**
 * get_node_type reads the first byte of a node to determine if it is leaf or internal.
 * C++ specific: uint8_t is used for the type field to save space.
 */
NodeType get_node_type(void* node) {
    uint8_t value;
    memcpy(&value, (char*)node + NODE_TYPE_OFFSET, NODE_TYPE_SIZE);
    return (NodeType)value;
}

void set_node_type(void* node, NodeType type) {
    uint8_t value = type;
    memcpy((char*)node + NODE_TYPE_OFFSET, &value, NODE_TYPE_SIZE);
}

bool is_node_root(void* node) {
    uint8_t value;
    memcpy(&value, (char*)node + IS_ROOT_OFFSET, IS_ROOT_SIZE);
    return (bool)value;
}

void set_node_root(void* node, bool is_root) {
    uint8_t value = is_root;
    memcpy((char*)node + IS_ROOT_OFFSET, &value, IS_ROOT_SIZE);
}

uint32_t* node_parent(void* node) {
    return (uint32_t*)((char*)node + PARENT_POINTER_OFFSET);
}

/*
 * Leaf Node Accessors
 */

uint32_t* leaf_node_num_cells(void* node) {
    return (uint32_t*)((char*)node + LEAF_NODE_NUM_CELLS_OFFSET);
}

uint32_t* leaf_node_next_leaf(void* node) {
    return (uint32_t*)((char*)node + LEAF_NODE_NEXT_LEAF_OFFSET);
}

/**
 * leaf_node_cell returns a pointer to a specific cell in a leaf node.
 * C++ specific: Pointer arithmetic on void* is not allowed, so we cast to char*.
 */
void* leaf_node_cell(void* node, uint32_t cell_num) {
    return (char*)node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

uint32_t* leaf_node_key(void* node, uint32_t cell_num) {
    return (uint32_t*)leaf_node_cell(node, cell_num);
}

void* leaf_node_value(void* node, uint32_t cell_num) {
    return (char*)leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

void initialize_leaf_node(void* node) {
    set_node_type(node, NODE_LEAF);
    set_node_root(node, false);
    *leaf_node_num_cells(node) = 0;
    *leaf_node_next_leaf(node) = 0;
}

/*
 * Internal Node Accessors
 */

uint32_t* internal_node_num_keys(void* node) {
    return (uint32_t*)((char*)node + INTERNAL_NODE_NUM_KEYS_OFFSET);
}

uint32_t* internal_node_right_child(void* node) {
    return (uint32_t*)((char*)node + INTERNAL_NODE_RIGHT_CHILD_OFFSET);
}

uint32_t* internal_node_cell(void* node, uint32_t cell_num) {
    return (uint32_t*)((char*)node + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE);
}

uint32_t* internal_node_child(void* node, uint32_t child_num) {
    uint32_t num_keys = *internal_node_num_keys(node);
    if (child_num > num_keys) {
        std::cerr << "Tried to access child_num " << child_num << " > num_keys " << num_keys << std::endl;
        exit(EXIT_FAILURE);
    } else if (child_num == num_keys) {
        return internal_node_right_child(node);
    } else {
        return (uint32_t*)internal_node_cell(node, child_num);
    }
}

uint32_t* internal_node_key(void* node, uint32_t key_num) {
    return (uint32_t*)((char*)internal_node_cell(node, key_num) + INTERNAL_NODE_CHILD_SIZE);
}

void initialize_internal_node(void* node) {
    set_node_type(node, NODE_INTERNAL);
    set_node_root(node, false);
    *internal_node_num_keys(node) = 0;
}

/**
 * get_node_max_key returns the largest key stored in the given node.
 * For internal nodes, it's the rightmost key. For leaf nodes, it's the last cell's key.
 */
uint32_t get_node_max_key(void* node) {
    switch (get_node_type(node)) {
        case NODE_INTERNAL:
            return *internal_node_key(node, *internal_node_num_keys(node) - 1);
        case NODE_LEAF:
            return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
    }
    return 0;
}

void update_internal_node_key(void* node, uint32_t old_key, uint32_t new_key) {
    uint32_t num_keys = *internal_node_num_keys(node);
    for (uint32_t i = 0; i < num_keys; i++) {
        if (*internal_node_key(node, i) == old_key) {
            *internal_node_key(node, i) = new_key;
            return;
        }
    }
}

/**
 * create_new_root handles the split of the root node.
 * It creates a new internal node and sets the old root as its first child.
 */
void create_new_root(Table* table, uint32_t right_child_page_num) {
    void* root = table->pager->get_page(table->root_page_num);
    void* right_child = table->pager->get_page(right_child_page_num);
    uint32_t left_child_page_num = table->pager->get_unused_page_num();
    void* left_child = table->pager->get_page(left_child_page_num);

    // Old root is copied to new left child
    memcpy(left_child, root, PAGE_SIZE);
    set_node_root(left_child, false);

    // Root becomes an internal node with two children
    initialize_internal_node(root);
    set_node_root(root, true);
    *internal_node_num_keys(root) = 1;
    *internal_node_child(root, 0) = left_child_page_num;
    uint32_t left_child_max_key = get_node_max_key(left_child);
    *internal_node_key(root, 0) = left_child_max_key;
    *internal_node_right_child(root) = right_child_page_num;
    *node_parent(left_child) = table->root_page_num;
    *node_parent(right_child) = table->root_page_num;
}
