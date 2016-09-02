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
constexpr int INT_SIZE = sizeof(int);
const static string NULL_STR = "NULL";

struct fast_hash_func {
    size_t operator()(const string &__s) const noexcept {
        return std::_Hash_impl::hash(__s.data(), 8);
    }
};

fast_hash_func hash_func;

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

    string get(const string &key) {
        auto index = hash_func(key) % max_slot_size_;
        for (; hash_table_[index].key_str_.size() != 0; index = (index + 1) % max_slot_size_) {
            if (hash_table_[index].key_str_ == key) {
                return hash_table_[index].value_str;
            }
        }
        return NULL_STR;
    }

    void put(string &key, string &value) {
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
    char *mmap_buffer_;
    int fd_;
    int index_{0};
    int integer;
    string key_;
    string value_;
    int key_len_;
    int val_len_;

public:
    Answer() {
        map_.reserve(48000);
        fd_ = open(DB_NAME, O_RDWR | O_CREAT, 0600);
        struct stat st;
        fstat(fd_, &st);
        if (st.st_size != SMALL_SIZE)
            ftruncate(fd_, SMALL_SIZE);
        mmap_buffer_ = (char *) mmap(NULL, SMALL_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED | MAP_POPULATE, fd_, 0);
        for (;;) {
            memcpy(&integer, mmap_buffer_ + index_, INT_SIZE);
            key_len_ = integer;
            if (key_len_ == 0)
                break;
            index_ += INT_SIZE;
            if (key_len_ != 0) {
                key_.assign(mmap_buffer_ + index_, key_len_);
                index_ += key_len_;
                memcpy(&integer, mmap_buffer_ + index_, INT_SIZE);
                val_len_ = integer;
                index_ += INT_SIZE;
                value_.assign(mmap_buffer_ + index_, val_len_);
                index_ += val_len_;
                map_.put(key_, value_);
            }
        }
    }

    string get(string key) {
        return map_.get(key);
    }

    void put(string key, string value) {
        key_len_ = key.size();
        memcpy(mmap_buffer_ + index_, &key_len_, INT_SIZE);
        index_ += INT_SIZE;
        memcpy(mmap_buffer_ + index_, key.c_str(), key_len_);
        index_ += key_len_;
        val_len_ = value.size();
        memcpy(mmap_buffer_ + index_, &val_len_, INT_SIZE);
        index_ += INT_SIZE;
        memcpy(mmap_buffer_ + index_, value.c_str(), val_len_);
        index_ += val_len_;
        map_.put(key, value);
    }
};

#endif //KEYVALUESTORE_ANOTHER_IMPL_KEY_VALUE_H