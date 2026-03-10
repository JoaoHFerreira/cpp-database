#include "Cursor.h"
#include "Node.h"

void Cursor::leaf_node_find(Table* table, uint32_t page_num, uint32_t key) {
    void* node = table->pager->get_page(page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);

    // Binary search
    uint32_t min_index = 0;
    uint32_t one_past_max_index = num_cells;
    while (one_past_max_index != min_index) {
        uint32_t index = min_index + (one_past_max_index - min_index) / 2;
        uint32_t key_at_index = *leaf_node_key(node, index);
        if (key == key_at_index) {
            this->cell_num = index;
            return;
        }
        if (key < key_at_index) {
            one_past_max_index = index;
        } else {
            min_index = index + 1;
        }
    }
    this->cell_num = min_index;
}

uint32_t Cursor::internal_node_find_child(void* node, uint32_t key) {
    uint32_t num_keys = *internal_node_num_keys(node);
    uint32_t min_index = 0;
    uint32_t max_index = num_keys; // there is one more child than keys

    while (min_index != max_index) {
        uint32_t index = min_index + (max_index - min_index) / 2;
        uint32_t key_to_to_right = *internal_node_key(node, index);
        if (key_to_to_right >= key) {
            max_index = index;
        } else {
            min_index = index + 1;
        }
    }
    return *internal_node_child(node, min_index);
}

void Cursor::tree_find(Table* table, uint32_t page_num, uint32_t key) {
    void* node = table->pager->get_page(page_num);
    NodeType type = get_node_type(node);
    if (type == NODE_LEAF) {
        this->page_num = page_num;
        leaf_node_find(table, page_num, key);
    } else {
        uint32_t child_page_num = internal_node_find_child(node, key);
        tree_find(table, child_page_num, key);
    }
}

Cursor::Cursor(Table* table, uint32_t key) : table(table) {
    page_num = table->root_page_num;
    tree_find(table, page_num, key);
    void* node = table->pager->get_page(page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);
    end_of_table = (cell_num == num_cells);
}

Cursor::Cursor(Table* table, bool start) : table(table) {
    if (start) {
        // Find the leftmost leaf
        page_num = table->root_page_num;
        void* node = table->pager->get_page(page_num);
        while (get_node_type(node) == NODE_INTERNAL) {
            page_num = *internal_node_child(node, 0);
            node = table->pager->get_page(page_num);
        }
        cell_num = 0;
        end_of_table = (*leaf_node_num_cells(node) == 0);
    } else {
        page_num = table->root_page_num;
        void* node = table->pager->get_page(page_num);
        while (get_node_type(node) == NODE_INTERNAL) {
            page_num = *internal_node_right_child(node);
            node = table->pager->get_page(page_num);
        }
        cell_num = *leaf_node_num_cells(node);
        end_of_table = true;
    }
}

void* Cursor::value() {
    void* page = table->pager->get_page(page_num);
    return leaf_node_value(page, cell_num);
}

void Cursor::advance() {
    void* node = table->pager->get_page(page_num);
    cell_num += 1;
    if (cell_num >= (*leaf_node_num_cells(node))) {
        uint32_t next_page_num = *leaf_node_next_leaf(node);
        if (next_page_num == 0) {
            end_of_table = true;
        } else {
            page_num = next_page_num;
            cell_num = 0;
        }
    }
}
