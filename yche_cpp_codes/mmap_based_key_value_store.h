//
// Created by cheyulin on 8/21/16.
//

#ifndef KEYVALUESTORE_MMAP_BASED_KEY_VALUE_STORE_H
#define KEYVALUESTORE_MMAP_BASED_KEY_VALUE_STORE_H

#include <unordered_map>
#include <algorithm>
#include <string>
#include <cstring>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define INDEX_FILE_NAME "index.meta"
#define DB_NAME "value.db"
#define SMALL_INDEX_LENGTH 90000*77

using namespace std;

template<typename T>
void serialize(char *buffer, T integer) {
    memcpy(buffer, &integer, sizeof(T));
}

template<typename T>
T deserialize(char *buffer) {
    T integer;
    memcpy(&integer, buffer, sizeof(T));
}

void trim_right_blank(string &to_trim_string) {
    auto iter_back = std::find_if_not(to_trim_string.rbegin(), to_trim_string.rend(),
                                      [](int c) { return std::isspace(c); }).base();
    to_trim_string = std::string(to_trim_string.begin(), iter_back);
}

class Answer {
private:
    unordered_map<string, pair<uint32_t, uint16_t >> key_index_info_map_;
    char *key_index_mmap_;
    char *db_value_mmap_;
    int key_index_file_descriptor_;
    uint32_t prefix_sum_index_{0};
    uint16_t length_{0};

    inline void read_index_info() {
        key_index_file_descriptor_ = open(INDEX_FILE_NAME, O_RDWR | O_CREAT, 0600);
        ftruncate(key_index_file_descriptor_, SMALL_INDEX_LENGTH);
        key_index_mmap_ = (char *) mmap(0, SMALL_INDEX_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED,
                                        key_index_file_descriptor_, 0);
        string key_str;
        for (auto i = 0; i < 90000; ++i) {
            if (key_index_mmap_[i * 77 + 76] == '\n') {
                key_str = string(key_index_mmap_, i * 77, 70);
                trim_right_blank(key_str);
                prefix_sum_index_ = deserialize<uint32_t>(key_index_mmap_ + i * 77 + 70);
                length_ = deserialize<uint16_t>(key_index_mmap_ + i * 77 + 74);
                key_index_info_map_[key_str] = make_pair(index, length_);
            } else {
                break;
            }
        }
        prefix_sum_index_ = prefix_sum_index_ + length_;
    }

    inline void read_db_info() {

    }

public:
    Answer() {
        key_index_info_map_.reserve(18000);
        read_index_info();
        read_db_info();
    }

    virtual ~Answer() {
        munmap(key_index_mmap_, SMALL_INDEX_LENGTH);
        close(key_index_file_descriptor_);
    }

    inline string get(string key) {
        if (key_index_info_map_.find(key) == key_index_info_map_.end()) {
            return "NULL";
        }
        else {
            auto &index_pair = key_index_info_map_[key];
            return string();
        }
    }

    inline void put(string key, string value) {
        auto value_size = value.size();

        key_index_info_map_[key] = make_pair(prefix_sum_index_, value_size);
        prefix_sum_index_ += value_size;
    }
};

#endif //KEYVALUESTORE_MMAP_BASED_KEY_VALUE_STORE_H
