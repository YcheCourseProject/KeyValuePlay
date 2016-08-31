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

using namespace std;

#define FILE_NAME  "transaction.db"

hash<string> hash_func;

inline size_t get_file_size(int file_descriptor) {
    struct stat st;
    fstat(file_descriptor, &st);
    return st.st_size;
}

struct key_value_info {
    string key_str_{""};
    string value_str{""};
};

template<size_t slot_num = 200000>
class yche_map {
private:
    vector<key_value_info> hash_table_;
    size_t max_slot_size_{slot_num};

public:
    inline yche_map() : hash_table_(slot_num) {}

    inline void reserve(int size) {
        hash_table_.resize(size);
        max_slot_size_ = size;
    }

    inline string *find(const string &key) {
        auto index = hash_func(key) % max_slot_size_;
        for (; hash_table_[index].key_str_.size() != 0; index = (index + 1) % max_slot_size_) {
            if (hash_table_[index].key_str_ == key) {
                return &hash_table_[index].value_str;
            }
        }
        return nullptr;
    }

    inline void insert_or_replace(const string &key, const string &value) {
        auto index = hash_func(key) % max_slot_size_;
        for (; hash_table_[index].key_str_.size() != 0; index = (index + 1) % max_slot_size_) {
            if (hash_table_[index].key_str_ == key) {
                hash_table_[index].value_str = value;
                return;
            }
        }
        hash_table_[index].key_str_ = key;
        hash_table_[index].value_str = value;
    }
};

class Answer {
private:
    yche_map<48000> map_;
    int file_descriptor_;
    char *mmap_;
    int index_{0};

public:
    inline Answer() {
        fstream input_file_stream{FILE_NAME, ios::in | ios::binary};
        string key_str;
        string value_str;
        for (; input_file_stream.good();) {
            getline(input_file_stream, key_str);
            if (input_file_stream.good()) {
                getline(input_file_stream, value_str);
                map_.insert_or_replace(key_str, value_str);
                index_ += key_str.size() + value_str.size() + 2;
            }
        }
        file_descriptor_ = open(FILE_NAME, O_RDWR | O_CREAT, 0600);
        if (get_file_size(file_descriptor_) != 6000000)
            ftruncate(file_descriptor_, 6000000);
        mmap_ = (char *) mmap(0, 6000000, PROT_WRITE, MAP_SHARED, file_descriptor_, 0);
    }


    inline string get(string key) {
        auto result = map_.find(key);
        if (result != nullptr) {
            return *result;
        }
        else {
            return "NULL";
        }
    }

    inline void put(string key, string value) {
        memcpy(mmap_ + index_, key.c_str(), key.size());
        index_ += key.size();
        mmap_[index_] = '\n';
        ++index_;
        memcpy(mmap_ + index_, value.c_str(), value.size());
        index_ += value.size();
        mmap_[index_] = '\n';
        ++index_;
        map_.insert_or_replace(key, value);
    }
};

#endif //KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H
