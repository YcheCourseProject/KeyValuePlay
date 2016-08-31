//
// Created by cheyulin on 8/27/16.
//

#ifndef KEYVALUESTORE_CORRECT_FINAL_VERSION_KEY_VALUE_H
#define KEYVALUESTORE_CORRECT_FINAL_VERSION_KEY_VALUE_H

#include <algorithm>
#include <string>
#include <cstring>
#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include "unistd.h"
#include "fcntl.h"

#define DB_NAME "yche.db"
#define SMALL 6000000
#define MEDIUM 1500000000
#define LARGE 3000000000

using namespace std;
fstream file_stream_;
constexpr int INT_SIZE = sizeof(int);
char *buffer;

hash<string> hash_func;

struct [[pack]] key_value_info {
    string key_str_;
    string value_str_;
    int value_index_{0};
    int value_length_{0};
};

class yche_map {
private:
    vector<key_value_info> hash_table_;
    int max_slot_size_{0};
    string result_str_;

public:
    void resize(int size) {
        max_slot_size_ = size;
        hash_table_.resize(size);
    }

    string *find(const string &key) {
        auto index = hash_func(key) % max_slot_size_;
        for (; hash_table_[index].key_str_.size() != 0; index = (index + 1) % max_slot_size_) {
            if (hash_table_[index].key_str_ == key) {
                file_stream_.seekg(hash_table_[index].value_index_);
                file_stream_.read(buffer, hash_table_[index].value_length_);
                result_str_ = string(buffer, 0, hash_table_[index].value_length_);
                return &result_str_;
            }
        }
        return nullptr;
    }

    void insert_or_replace(const string &key, int value_index, int value_length) {
        auto index = hash_func(key) % max_slot_size_;
        for (; hash_table_[index].key_str_.size() != 0; index = (index + 1) % max_slot_size_) {
            if (hash_table_[index].key_str_ == key) {
                hash_table_[index].value_index_ = value_index;
                hash_table_[index].value_length_ = value_length;
                return;
            }
        }
        hash_table_[index].key_str_ = move(key);
        hash_table_[index].value_index_ = value_index;
        hash_table_[index].value_length_ = value_length;
    }
};

class Answer {
private:
    yche_map map_;
    string key_;
    string value_;
    int file_index_{0};
    int key_len_;
    int val_index_{0};
    int val_len_{0};
    bool is_init_{false};
    bool is_file_created{false};
    int integer_;

    void deserialize() {
        memcpy(&integer_, buffer, INT_SIZE);
    }

    void init_map() {
        if (!is_init_) {
            if (val_len_ <= 160) {
                map_.resize(50000);
            } else if (val_len_ <= 3000) {
                map_.resize(500000);
            }
            else {
                map_.resize(50000);
            }
            is_init_ = true;
        }
    }

    void init_file(int size) {
        int fd = open(DB_NAME, O_RDWR | O_CREAT, 0600);
        if (size <= 160) {
            cout << "truncate" << endl;
            ftruncate(fd, SMALL);
        } else if (size <= 30000) {
            ftruncate(fd, MEDIUM);
        } else {
            ftruncate(fd, LARGE);
        }
        is_file_created = true;
        file_stream_.open(DB_NAME, ios::in | ios::out);
        file_stream_.clear();
        file_stream_.seekp(0, ios::beg);
    }

public:
    Answer() {
        buffer = new char[32 * 1024];
        file_stream_.open(DB_NAME, ios::in | ios::out);
        if (file_stream_.good()) {
            is_file_created = true;
            for (;;) {
                file_stream_.read(buffer, INT_SIZE);
                deserialize();
                key_len_ = integer_;
                if (key_len_ == 0)
                    break;
                file_index_ += INT_SIZE;
                if (key_len_ != 0) {
                    file_stream_.read(buffer, key_len_);
                    key_.assign(buffer, key_len_);
                    file_index_ += key_len_;
                    file_stream_.read(buffer, INT_SIZE);
                    deserialize();
                    val_len_ = integer_;
                    file_index_ += INT_SIZE;
                    value_.assign(buffer, val_len_);
                    map_.insert_or_replace(key_, file_index_, val_len_);
                    file_index_ += val_len_;
                }
            }
            file_stream_.seekp(file_index_, ios::beg);
        }
    }

    string get(string key) {
        auto str_ptr = map_.find(key);
        if (str_ptr == nullptr) {
            return "NULL";
        }
        else {
            return *str_ptr;
        }
    }

    void put(string key, string value) {
        init_map();
        [[unlikely(true)]]
        if (!is_file_created)
            init_file(value.size());
        key_len_ = key.size();
        memcpy(buffer, &key_len_, INT_SIZE);
        if (file_stream_.good()) {
            cout << "ok" << endl;
        } else {
            cout << "bad" << endl;
        }
        file_stream_.write(buffer, INT_SIZE);
        file_stream_.write(key.c_str(), key.size());
        val_len_ = value.size();
        memcpy(buffer, &val_len_, INT_SIZE);
        file_stream_.write(buffer, INT_SIZE);
        file_stream_.write(value.c_str(), val_len_);
        file_stream_.flush();
        map_.insert_or_replace(key, file_index_ + key_len_ + INT_SIZE + INT_SIZE, val_len_);
        file_index_ += key_len_ + val_len_ + INT_SIZE + INT_SIZE;
    }
};

#endif //KEYVALUESTORE_CORRECT_FINAL_VERSION_KEY_VALUE_H
