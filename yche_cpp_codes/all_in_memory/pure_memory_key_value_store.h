//
// Created by cheyulin on 8/10/16.
//

#ifndef KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H
#define KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H

#include <string>
#include <fstream>
#include <algorithm>
#include <cstddef>

#include <cstring>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

#define FILE_NAME  "tuple_transaction.db"

hash<string> hash_func;

template<size_t slot_num = 200000>
class yche_map {
private:
    vector<pair<string, string>> my_hash_table_;
    size_t current_size_{0};
    size_t slot_max_size_{slot_num};

public:
    inline yche_map() : my_hash_table_(slot_num) {}

    inline void reserve(int size) {
        my_hash_table_.resize(size);
    }

    inline size_t size() {
        return current_size_;
    }

    inline string *find(const string &key) {
        auto index = hash_func(key) % slot_max_size_;
        //linear probing
        for (; my_hash_table_[index].first.size() != 0; index = (index + 1) % slot_max_size_) {
            if (my_hash_table_[index].first == key) {
                return &my_hash_table_[index].second;
            }
        }
        return nullptr;
    }

    inline void insert_or_replace(const string &key, const string &value) {
        auto index = hash_func(key) % slot_max_size_;
        for (; my_hash_table_[index].first.size() != 0; index = (index + 1) % slot_max_size_) {
            if (my_hash_table_[index].first == key) {
                my_hash_table_[index].second = value;
                return;
            }
        }
        ++current_size_;
        my_hash_table_[index].first = key;
        my_hash_table_[index].second = value;

    }
};

class Answer {
private:
    yche_map<50000> yche_map_;
    int file_descriptor_;
    char *mmap_;
    int index_{0};

public:
    inline Answer() {
        fstream input_file_stream{FILE_NAME, ios::in | ios::binary};
        bool is_first_in = true;
        string key_str;
        string value_str;
        for (; input_file_stream.good();) {
            getline(input_file_stream, key_str);
            if (input_file_stream.good()) {
                if (is_first_in)
                    is_first_in = false;
                getline(input_file_stream, value_str);
                yche_map_.insert_or_replace(key_str, value_str);
                index_ += key_str.size() + value_str.size() + 2;
            }
        }
        file_descriptor_ = open(FILE_NAME, O_RDWR | O_CREAT, 0600);
        if (is_first_in)
            ftruncate(file_descriptor_, 6000000);
        mmap_ = (char *) mmap(0, 6000000, PROT_WRITE, MAP_SHARED, file_descriptor_, 0);
    }

    inline string get(string key) {
        auto result = yche_map_.find(key);
        if (result != nullptr) {
            return *result;
        }
        else {
            return "NULL";
        }
    }

    inline void put(string key, string value) {
        memcpy(mmap_ + index_, key.c_str(), key.size());
        mmap_[index_ + key.size()] = '\n';
        memcpy(mmap_ + index_ + key.size() + 1, value.c_str(), value.size());
        mmap_[index_ + key.size() + value.size() + 1] = '\n';
        index_ += key.size() + value.size() + 2;
        yche_map_.insert_or_replace(key, value);
    }
};

#endif //KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H