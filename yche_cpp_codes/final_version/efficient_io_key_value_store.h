//
// Created by cheyulin on 8/20/16.
//

#ifndef KEYVALUESTORE_EFFICIENT_IO_KEY_VALUE_STORE_H
#define KEYVALUESTORE_EFFICIENT_IO_KEY_VALUE_STORE_H

#include <unordered_map>
#include <algorithm>
#include <string>
#include <fstream>

#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#define INDEX_FILE_NAME "index.meta"
#define DB_NAME "value.db"

using namespace std;

inline size_t get_file_size(int file_descriptor) {
    struct stat st;
    fstat(file_descriptor, &st);
    return st.st_size;
}

hash<string> hash_func;

template<typename T, size_t slot_num = 900000>
class yche_map {
private:
    vector<pair<string, T>> my_hash_table_;
    size_t current_size_{0};
    size_t slot_max_size_{slot_num};

public:
    inline yche_map() : my_hash_table_(slot_num) {}

    inline T *find(const string &key) {
        auto index = hash_func(key) % slot_max_size_;
        for (; my_hash_table_[index].first.size() != 0; index = (index + 1) % slot_max_size_) {
            if (my_hash_table_[index].first == key) {
                return &my_hash_table_[index].second;
            }
        }
        return nullptr;
    }

    inline void insert_or_replace(const string &key, const T &value) {
        auto index = hash_func(key) % slot_max_size_;
        for (; my_hash_table_[index].first.size() != 0; index = (index + 1) % slot_max_size_) {
            if (my_hash_table_[index].first == key) {
                my_hash_table_[index].second = value;
                return;
            }
        }
        my_hash_table_[index].first = key;
        my_hash_table_[index].second = value;
    }
};

class Answer {
private:
    yche_map<pair<int, int>> yche_map_;
    fstream key_index_stream_;
    fstream db_stream_;
    int db_file_descriptor_;
    char *db_mmap_;
    char *db_read_mmap_;
    int prefix_sum_index_{0};
    int length_{0};
    char *value_buffer;

    bool is_initialized_{false};

    inline void read_index_info() {
        string key_str;
        string prefix_sum_index_str;
        string length_str;
        for (; key_index_stream_.good();) {
            getline(key_index_stream_, key_str);
            if (key_index_stream_.good()) {
                getline(key_index_stream_, prefix_sum_index_str);
                getline(key_index_stream_, length_str);
                prefix_sum_index_ = stoi(prefix_sum_index_str);
                length_ = stoi(length_str);
                yche_map_.insert_or_replace(key_str, make_pair(prefix_sum_index_, length_));
            }
        }
        prefix_sum_index_ = prefix_sum_index_ + length_;
        key_index_stream_.clear();
    }

public:
    Answer() {
        value_buffer = new char[1024 * 32];
        key_index_stream_.open(INDEX_FILE_NAME, ios::in | ios::out | ios::app | ios::binary);
        db_file_descriptor_ = open(DB_NAME, O_RDWR | O_CREAT, 0600);
        db_stream_.open(DB_NAME, ios::in | ios::binary);
        read_index_info();
    }

    virtual ~Answer() {
        delete[] value_buffer;
    }

    inline string get(string key) {
        if (yche_map_.find(key) == nullptr) {
            return "NULL";
        }
        else {
            auto *index_pair = yche_map_.find(key);
            db_stream_.seekg(index_pair->first, ios::beg);
            db_stream_.read(value_buffer, index_pair->second);
            return string(value_buffer, 0, index_pair->second);
        }
    }

    inline void put(string key, string value) {
        length_ = value.size();
        if (!is_initialized_) {
            unsigned long file_size = get_file_size(db_file_descriptor_);
            if (file_size < 10) {
                if (length_ < 500) {
                    ftruncate(db_file_descriptor_, 10000000);
                    file_size = 10000000;
                } else if (length_ < 5000) {
                    ftruncate(db_file_descriptor_, 1600000000);
                    file_size = 1600000000;
                } else {
                    ftruncate(db_file_descriptor_, 2900000000);
                    file_size = 2900000000;
                }
            }
            db_mmap_ = (char *) mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, db_file_descriptor_, 0);
            db_read_mmap_ = (char *) mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, db_file_descriptor_, 0);
            is_initialized_ = true;
        }
        key_index_stream_ << key << "\n";
        key_index_stream_ << prefix_sum_index_ << "\n";
        key_index_stream_ << length_ << "\n" << flush;

        memcpy(db_mmap_ + prefix_sum_index_, value.c_str(), length_);

        yche_map_.insert_or_replace(key, make_pair(prefix_sum_index_, length_));
        prefix_sum_index_ += length_;
    }
};

#endif //KEYVALUESTORE_EFFICIENT_IO_KEY_VALUE_STORE_H