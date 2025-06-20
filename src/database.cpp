#include "database.hpp"
#include <iostream>

Database::Database() {
    keyValueStore = wal.loadFromWal();
}

std::string Database::processInput(std::string command) {
    parser.tokenizerInput(command);
    dbMsg = dispatchCommand();
    parser.tokens.clear();
    wal.flushWalToDisk();
    return dbMsg;
}

std::string Database::dispatchCommand() {
    ActionOptions actions;
    dbMsg = parser.validateCommand();
    if (dbMsg == "")
    {
        if (parser.action == actions.set) {
            dbMsg = set();
            return dbMsg;
        }
        if (parser.action == actions.get) {
            dbMsg = get();
            return dbMsg;
        }
        if (parser.action == actions.del) {
            dbMsg = del();
            return dbMsg;
        }
        if (parser.action == actions.list) {
            dbMsg = list();
            return dbMsg;
        } 
    }
    
    return dbMsg;
}

std::string Database::set() {
    if (!parser.validateSetSyntax()) {
        return "Parsing Error: Must have 3 Elements SET and key and value. eg; SET key value;\n";
    }

    std::lock_guard<std::mutex> lock(dbMutex);

    key = parser.tokens[1];
    value = parser.tokens.back();
    // keyValueStore[key] = value;
    wal.appendWalEntry("SET", key, value);

    std::vector<char> entry;
    uint32_t key_len = key.size();
    uint32_t val_len = value.size();

    std::cout << "[DEBUG] Inserting key: \"" << key << "\" (len=" << key_len << "), value: \"" << value << "\" (len=" << val_len << ")\n";

    entry.resize(Page::HEADER_SIZE + key_len + val_len);
    std::memcpy(entry.data(), &key_len, Page::KEY_LEN_SIZE);
    std::memcpy(entry.data() + Page::KEY_LEN_SIZE, &val_len, Page::VAL_LEN_SIZE);
    std::memcpy(entry.data() + Page::HEADER_SIZE, key.data(), key_len);
    std::memcpy(entry.data() + Page::HEADER_SIZE + key_len, value.data(), val_len);

    std::cout << "[DEBUG] Entry size: " << entry.size() << " bytes\n";

    bool written = false;
    for (auto& [id, page] : pageManager.pageTable) {
        size_t offset = 0;

        while (offset + entry.size() <= PAGE_SIZE) {
            uint32_t existing_key_len = 0;
            std::memcpy(&existing_key_len, page.data.data() + offset, Page::KEY_LEN_SIZE);

            std::cout << "[DEBUG] Page " << id << ", offset " << offset << ": existing_key_len = " << existing_key_len << "\n";

            if (existing_key_len == 0) {
                std::memcpy(page.data.data() + offset, entry.data(), entry.size());
                page.dirty = true;
                written = true;
                std::cout << "[DEBUG] Inserted entry at page " << id << ", offset " << offset << "\n";
                break;
            }

            uint32_t existing_val_len = 0;
            std::memcpy(&existing_val_len, page.data.data() + offset + Page::KEY_LEN_SIZE, Page::VAL_LEN_SIZE);
            offset += Page::HEADER_SIZE + existing_key_len + existing_val_len;
        }

        if (written) break;
    }

    if (!written) {
        // Use PageManager's allocatePage method instead of manual ID assignment
        Page* newPage = pageManager.allocatePage();
        std::memcpy(newPage->data.data(), entry.data(), entry.size());
        newPage->dirty = true;

        std::cout << "[DEBUG] Created new page with id = " << newPage->id << ", inserted entry at offset 0\n";
    }

    // CRITICAL: Flush the dirty pages to disk
    pageManager.flushAll();

    return "Value inserted!\n";
}


std::string Database::get() {
    if (!parser.validateGetSyntax()) {
        return "Parsing Error: Must have 2 Elements GET and key to get. eg; GET my_key\n";
    }
    
    std::lock_guard<std::mutex> lock(dbMutex);
    key = parser.tokens.back();
    key.pop_back();
    wal.appendWalEntry("GET", key, keyValueStore[key]);
    
    std::ifstream file("data.db", std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return "Error opening database file\n";
    }
    
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    const size_t totalPages = fileSize / PAGE_SIZE;
    std::array<char, PAGE_SIZE> buffer;
    
    for (size_t i = 0; i < totalPages; i++) {
        if (!file.read(buffer.data(), PAGE_SIZE)) {
            return "Failed to read page " + std::to_string(i) + "\n";
        }
        
        size_t offset = 0;
        while (offset + Page::HEADER_SIZE <= PAGE_SIZE) {
            uint32_t key_len = 0;
            std::memcpy(&key_len, &buffer[offset], Page::KEY_LEN_SIZE);
            
            if (key_len == 0) {
                std::cout << "[DEBUG GET] Found zero key_len on disk, breaking\n";
                break;
            }
            
            uint32_t val_len = 0;
            std::memcpy(&val_len, &buffer[offset + Page::KEY_LEN_SIZE], Page::VAL_LEN_SIZE);
            std::cout << "[DEBUG GET] Value length from disk: " << val_len << "\n";
            
            // Check if we have enough space for the entire entry
            size_t total_entry_size = Page::HEADER_SIZE + key_len + val_len;
            if (offset + total_entry_size > PAGE_SIZE) {
                std::cout << "[DEBUG GET] Not enough space for entire entry on disk, breaking\n";
                break;
            }
            
            std::string keyStr(&buffer[offset + Page::HEADER_SIZE], key_len);
            std::string valStr(&buffer[offset + Page::HEADER_SIZE + key_len], val_len);
            
            if (keyStr == key) {
                return keyStr + "\t\t" + valStr + "\n";
            }
            
            // CRITICAL: Advance offset to next entry
            offset += total_entry_size;
            std::cout << "[DEBUG GET] Next offset: " << offset << "\n";
        }
    }
    
    return "Value not found\n";
}


std::string Database::del() {
    if (!parser.validateDelSyntax()) {
        return "Parsing Error: Must have 2 Elements DEL and key to delete. eg; DEL my_\n";
    }

    std::lock_guard<std::mutex> lock(dbMutex);
    key = parser.tokens.back();
    key.pop_back();

    // Log to WAL
    wal.appendWalEntry("DEL", key, keyValueStore[key]);

    // In-memory delete
    keyValueStore.erase(key);

    std::fstream file("data.db", std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        return "Error opening database file\n";
    }

    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    const size_t totalPages = fileSize / PAGE_SIZE;

    std::array<char, PAGE_SIZE> buffer;

    for (size_t pageIndex = 0; pageIndex < totalPages; ++pageIndex) {
        std::streampos pagePos = pageIndex * PAGE_SIZE;
        file.seekg(pagePos);
        file.read(buffer.data(), PAGE_SIZE);

        size_t offset = 0;
        while (offset + Page::HEADER_SIZE <= PAGE_SIZE) {
            uint32_t key_len = 0;
            std::memcpy(&key_len, &buffer[offset], Page::KEY_LEN_SIZE);

            if (key_len == 0) {
                break; // tombstone or end of records
            }

            uint32_t val_len = 0;
            std::memcpy(&val_len, &buffer[offset + Page::KEY_LEN_SIZE], Page::VAL_LEN_SIZE);

            size_t total_entry_size = Page::HEADER_SIZE + key_len + val_len;
            if (offset + total_entry_size > PAGE_SIZE) {
                break;
            }

            std::string keyStr(&buffer[offset + Page::HEADER_SIZE], key_len);

            if (keyStr == key) {
                // Write tombstone
                uint32_t zero = 0;
                file.seekp(static_cast<std::streamoff>(pagePos) + static_cast<std::streamoff>(offset));
                file.write(reinterpret_cast<char*>(&zero), sizeof(uint32_t)); // key_len = 0
                file.flush();
                return "Register erased and tombstone written\n";
            }

            offset += total_entry_size;
        }
    }

    return "Key not found on disk\n";
}


std::string Database::list() {
    std::string result;
    std::lock_guard<std::mutex> lock(dbMutex);

    wal.appendWalEntry("LIST", "", "");
    int diskEntries = 0;
    std::ifstream file("data.db", std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        result += "Error opening database file\n";
        std::cout << "[DEBUG LIST] Failed to open data.db\n";
        return result;
    }
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    const size_t totalPages = fileSize / PAGE_SIZE;

    std::array<char, PAGE_SIZE> buffer;

    for (size_t i = 0; i < totalPages; ++i) {
        if (!file.read(buffer.data(), PAGE_SIZE)) {
            std::cout << "[DEBUG LIST] Failed to read page " << i << "\n";
            continue;
        }

        std::cout << "[DEBUG LIST] Reading disk page " << i << "\n";

        size_t offset = 0;
        while (offset + Page::HEADER_SIZE <= PAGE_SIZE) {
            // Read key length
            uint32_t key_len = 0;
            std::memcpy(&key_len, &buffer[offset], Page::KEY_LEN_SIZE);
            
            std::cout << "[DEBUG LIST] Disk page " << i << ", offset " << offset << ", key_len=" << key_len << "\n";
            
            if (key_len == 0) {
                std::cout << "[DEBUG LIST] Found zero key_len on disk, breaking\n";
                break;
            }

            // Read value length (it comes right after key_len, before the actual key data)  
            uint32_t val_len = 0;
            std::memcpy(&val_len, &buffer[offset + Page::KEY_LEN_SIZE], Page::VAL_LEN_SIZE);
            std::cout << "[DEBUG LIST] Value length from disk: " << val_len << "\n";

            // Check if we have enough space for the entire entry
            size_t total_entry_size = Page::HEADER_SIZE + key_len + val_len;
            if (offset + total_entry_size > PAGE_SIZE) {
                std::cout << "[DEBUG LIST] Not enough space for entire entry on disk, breaking\n";
                break;
            }

            // Extract key (starts after the header: key_len + val_len)
            std::string keyStr(&buffer[offset + Page::HEADER_SIZE], key_len);
            std::cout << "[DEBUG LIST] Extracted key from disk: '" << keyStr << "'\n";

            // Extract value (starts after header + key)
            std::string valStr(&buffer[offset + Page::HEADER_SIZE + key_len], val_len);
            std::cout << "[DEBUG LIST] Extracted value from disk: '" << valStr << "'\n";

            result += keyStr + "\t\t" + valStr + "\n";
            diskEntries++;
            std::cout << "[DEBUG LIST] Found on disk: key='" << keyStr << "', value='" << valStr << "'\n";

            // Move to next entry
            offset += total_entry_size;
            std::cout << "[DEBUG LIST] Next disk offset: " << offset << "\n";
        }
    }

    if (diskEntries == 0) {
        result += "No Registers\n";
    } else {
        result += "Finish Registers LIST!\n";
    }

    return result;
}