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
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

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
    yche_map<60000> yche_map_;
    int file_descriptor_;
    char *mmap_;
    int index_{0};

public:
    inline Answer() {
        struct stat file_status;
        file_descriptor_ = open(FILE_NAME, O_RDWR | O_CREAT, 0600);
        fstat(file_descriptor_, &file_status);
        ftruncate(file_descriptor_, 6000000);

        mmap_ = (char *) mmap(0, 6000000, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor_, 0);
        string key_string;
        string value_string;
        int start = 0;
        int end = 0;
        for (bool is_change = true; is_change;) {
            is_change = false;
            for (end = start; end - start < 71; ++end) {
                if (mmap_[end] == '\n') {
                    key_string = string(mmap_, start, end - start);
                    cout << "key:" << key_string << endl;
                    start = end + 1;
                    for (end = start; end - start < 261; ++end) {
                        if (mmap_[end] == '\n') {
                            value_string = string(mmap_, start, end - start);
                            cout << "value:" << value_string << endl;
                            yche_map_.insert_or_replace(key_string, value_string);
                            index_ += key_string.size() + value_string.size() + 2;
                            is_change = true;
                            start = end + 1;
                        }
                    }
                }
            }
        }
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