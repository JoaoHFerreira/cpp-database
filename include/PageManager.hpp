#pragma once
#include <array>
#include <unordered_map>
#include <fstream>


constexpr size_t PAGE_SIZE = 4096;

struct Page {
    uint32_t id = 0;
    bool dirty = false;
    std::array<char, PAGE_SIZE> data{};

    static constexpr size_t KEY_LEN_SIZE = 4;
    static constexpr size_t VAL_LEN_SIZE = 4;
    static constexpr size_t HEADER_SIZE = KEY_LEN_SIZE + VAL_LEN_SIZE;
};



class PageManager {
public:
    std::unordered_map<uint32_t, Page> pageTable;
    PageManager(const std::string& filename);
    Page* getPage(uint32_t pageId);
    Page* allocatePage();
    void flushPage(Page* page);
    void flushAll();

private:
    std::fstream file;
    uint32_t nextPageId = 0;
    std::string filename;
};
