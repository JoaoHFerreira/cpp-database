#include "PageManager.hpp"
#include <iostream>

PageManager::PageManager(const std::string& filename) : filename(filename) {
    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        file.open(filename, std::ios::out | std::ios::binary);
        file.close();
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    }
}

Page* PageManager::getPage(uint32_t pageId) {
    if (pageTable.find(pageId) != pageTable.end()) {
        return &pageTable[pageId];
    }
    Page newPage;
    newPage.id = pageId;
    file.seekg(pageId * PAGE_SIZE);
    file.read(newPage.data.data(), PAGE_SIZE);
    pageTable[pageId] = newPage;
    return &pageTable[pageId];
}

Page* PageManager::allocatePage() {
    Page page;
    page.id = nextPageId++;
    pageTable[page.id] = page;
    return &pageTable[page.id];
}

void PageManager::flushPage(Page* page) {
    if (!page->dirty) {
        std::cout << "[DEBUG FLUSH] Page " << page->id << " is not dirty, skipping\n";
        return;
    }
    
    std::cout << "[DEBUG FLUSH] Flushing page " << page->id << " to disk at position " << (page->id * PAGE_SIZE) << "\n";
    
    file.seekp(page->id * PAGE_SIZE);
    if (file.fail()) {
        std::cout << "[DEBUG FLUSH] ERROR: seekp failed for page " << page->id << "\n";
        return;
    }
    
    file.write(page->data.data(), PAGE_SIZE);
    if (file.fail()) {
        std::cout << "[DEBUG FLUSH] ERROR: write failed for page " << page->id << "\n";
        return;
    }
    
    page->dirty = false;
    std::cout << "[DEBUG FLUSH] Successfully flushed page " << page->id << "\n";
}

void PageManager::flushAll() {
    std::cout << "[DEBUG FLUSH] Flushing all pages, total pages: " << pageTable.size() << "\n";
    
    for (auto& [id, page] : pageTable) {
        flushPage(&page);
    }
    
    file.flush();
    if (file.fail()) {
        std::cout << "[DEBUG FLUSH] ERROR: file.flush() failed\n";
    } else {
        std::cout << "[DEBUG FLUSH] file.flush() successful\n";
    }
}