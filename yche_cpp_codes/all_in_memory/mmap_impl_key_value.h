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

inline void serialize(char *buffer, int integer) {
    memcpy(buffer, &integer, int_size);
}

inline int deserialize(char *buffer) {
    int integer;
    memcpy(&integer, buffer, int_size);
}

hash<string> hash_func;

struct key_value_info {
    string key_str_{""};
    string value_str{""};
};

template<size_t slot_num = 10>
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

    inline void insert_or_replace(const string &key, string &&value) {
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
    yche_map<> map_;
    char *mmap_;
    int fd_;
    int index_{0};
    string key_;
    string value_;
    int key_len_;
    int val_len_;
    char *buff_;

public:
    inline Answer() {
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
            key_len_ = deserialize(mmap_ + index_);
            if (key_len_ == 0)
                break;
            index_ += sizeof(int);
            if (key_len_ != 0) {
                key_.assign(mmap_ + index_, key_len_);
                index_ += key_len_;
                val_len_ = deserialize(mmap_ + index_);
                index_ += int_size;
                value_.assign(mmap_ + index_, val_len_);
                index_ += val_len_;
                map_.insert_or_replace(key_, move(value_));
            }
        }
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
        map_.insert_or_replace(key, move(value));
    }
};

#endif //KEYVALUESTORE_ANOTHER_IMPL_KEY_VALUE_H
