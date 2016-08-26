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
#define SMALL_INDEX_LENGTH 3000000
#define SMALL_DB_LENGTH 6000000

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
    unordered_map<string, pair<uint32_t, uint16_t >> yche_map_;
    char *key_index_mmap_;
    char *db_value_mmap_;
    char *serialization_buf_;

    int key_index_file_descriptor_;
    int db_file_descriptor_;

    int key_count_{0};
    uint32_t value_index_{0};
    uint16_t length_{0};

    inline void read_index_info() {
        key_index_file_descriptor_ = open(INDEX_FILE_NAME, O_RDWR | O_CREAT, 0600);
        ftruncate(key_index_file_descriptor_, SMALL_INDEX_LENGTH);
        key_index_mmap_ = (char *) mmap(0, SMALL_INDEX_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED,
                                        key_index_file_descriptor_, 0);
        madvise(key_index_mmap_, SMALL_INDEX_LENGTH, MADV_WILLNEED | MADV_SEQUENTIAL);
        string key_str;
        for (auto i = 0; i < 90000; ++i) {
            if (key_index_mmap_[i * 77 + 76] == '\n') {
                key_str = string(key_index_mmap_ + i * 77, 0, 70);
                trim_right_blank(key_str);
                if (key_str.length() == 0)
                    continue;
                key_count_++;
                value_index_ = deserialize<uint32_t>(key_index_mmap_ + i * 77 + 70);
                length_ = deserialize<uint16_t>(key_index_mmap_ + i * 77 + 74);
                yche_map_[key_str] = make_pair(value_index_, length_);
            } else {
                break;
            }
        }
        value_index_ = value_index_ + length_;
    }

    inline void read_db_info() {
        db_file_descriptor_ = open(DB_NAME, O_RDWR | O_CREAT, 0600);
        ftruncate(db_file_descriptor_, SMALL_DB_LENGTH);
        db_value_mmap_ = (char *) mmap(0, SMALL_DB_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, db_file_descriptor_, 0);
        madvise(db_value_mmap_, SMALL_DB_LENGTH, MADV_WILLNEED | MADV_RANDOM);
    }

public:
    Answer() {
        yche_map_.reserve(60000);
        serialization_buf_ = new char[4];
        read_index_info();
        read_db_info();
    }

    inline string get(string key) {
        if (yche_map_.find(key) == yche_map_.end()) {
            return "NULL";
        }
        else {
            auto index_pair = yche_map_[key];
            return string(db_value_mmap_ + index_pair.first, 0, index_pair.second);
        }
    }

    inline void put(string key, string value) {
        length_ = value.size();
        yche_map_[key] = make_pair(value_index_, length_);

        char *ptr = key_index_mmap_ + key_count_ * 77;
        memcpy(ptr, key.c_str(), key.size());
        serialize(serialization_buf_, value_index_);
        memcpy(ptr + 70, serialization_buf_, 4);
        serialize(serialization_buf_, length_);
        memcpy(ptr + 74, serialization_buf_, 2);
        ptr[76] = '\n';
        memcpy(db_value_mmap_ + value_index_, value.c_str(), length_);

        value_index_ += length_;
        ++key_count_;
    }
};

#endif //KEYVALUESTORE_MMAP_BASED_KEY_VALUE_STORE_H