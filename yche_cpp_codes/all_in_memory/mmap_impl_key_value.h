//
// Created by cheyulin on 8/26/16.
//
#ifndef KEYVALUESTORE_ANOTHER_IMPL_KEY_VALUE_H
#define KEYVALUESTORE_ANOTHER_IMPL_KEY_VALUE_H

#include <cstring>
#include <string>
#include <vector>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "fcntl.h"

#define DB_NAME "yche.db"
#define SMALL_SIZE 6000000

using namespace std;
constexpr int int_size = sizeof(int);
int integer;

void serialize(char *buffer, int my_integer) {
    memcpy(buffer, &my_integer, int_size);
}

void deserialize(char *buffer) {
    memcpy(&integer, buffer, int_size);
}

hash<string> hash_func;

struct key_value_info {
    string key_str_;
    string value_str;
};

class yche_map {
private:
    vector<key_value_info> hash_table_;
    size_t max_slot_size_{0};

public:
    void reserve(int size) {
        hash_table_.resize(size);
        max_slot_size_ = size;
    }

    string *find(const string &key) {
        auto index = hash_func(key) % max_slot_size_;
        for (; hash_table_[index].key_str_.size() != 0; index = (index + 1) % max_slot_size_) {
            if (hash_table_[index].key_str_ == key) {
                return &hash_table_[index].value_str;
            }
        }
        return nullptr;
    }

    void insert_or_replace(string &key, string &value) {
        auto index = hash_func(key) % max_slot_size_;
        for (; hash_table_[index].key_str_.size() != 0; index = (index + 1) % max_slot_size_) {
            if (hash_table_[index].key_str_ == key) {
                hash_table_[index].value_str = move(value);
                return;
            }
        }
        hash_table_[index].key_str_ = move(key);
        hash_table_[index].value_str = move(value);
    }
};

class Answer {
private:
    yche_map map_;
    char *mmap_;
    int fd_;
    int index_{0};
    string key_;
    string value_;
    int key_len_;
    int val_len_;
    char *buff_;

public:
    Answer() {
        map_.reserve(48000);
        buff_ = new char[10];
        fd_ = open(DB_NAME, O_RDWR | O_CREAT, 0600);
        struct stat st;
        fstat(fd_, &st);
        if (st.st_size != SMALL_SIZE)
            ftruncate(fd_, SMALL_SIZE);
        mmap_ = (char *) mmap(NULL, SMALL_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, fd_, 0);
        madvise(0, SMALL_SIZE, MADV_SEQUENTIAL | MADV_WILLNEED);
        for (;;) {
            deserialize(mmap_ + index_);
            key_len_ = integer;
            if (key_len_ == 0)
                break;
            index_ += int_size;
            if (key_len_ != 0) {
                key_.assign(mmap_ + index_, key_len_);
                index_ += key_len_;
                deserialize(mmap_ + index_);
                val_len_ = integer;
                index_ += int_size;
                value_.assign(mmap_ + index_, val_len_);
                index_ += val_len_;
                map_.insert_or_replace(key_, value_);
            }
        }
    }

    string get(string key) {
        auto result = map_.find(key);
        if (result != nullptr) {
            return *result;
        }
        else {
            return "NULL";
        }
    }

    void put(string key, string value) {
        serialize(buff_, key.size());
        memcpy(mmap_ + index_, buff_, int_size);
        index_ += int_size;
        memcpy(mmap_ + index_, key.c_str(), key.size());
        index_ += key.size();
        serialize(buff_, value.size());
        memcpy(mmap_ + index_, buff_, int_size);
        index_ += int_size;
        memcpy(mmap_ + index_, value.c_str(), value.size());
        index_ += value.size();
        map_.insert_or_replace(key, value);
    }
};

#endif //KEYVALUESTORE_ANOTHER_IMPL_KEY_VALUE_H