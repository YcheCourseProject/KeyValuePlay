//
// Created by cheyulin on 8/26/16.
//

#ifndef KEYVALUESTORE_ANOTHER_IMPL_KEY_VALUE_H
#define KEYVALUESTORE_ANOTHER_IMPL_KEY_VALUE_H

#include <cstring>
#include <string>
#include <unordered_map>
#include <sys/mman.h>
#include <sys/stat.h>
#include "fcntl.h"
#include <unistd.h>

#define DB_NAME "yche.db"
using namespace std;
constexpr int int_size = sizeof(int);

void serialize(char *buffer, int integer) {
    memcpy(buffer, &integer, int_size);
}

int deserialize(char *buffer) {
    int integer;
    memcpy(&integer, buffer, int_size);
}

class Answer {
private:
    char *mmap_;
    int fd_;
    int index_{0};
    int key_len_;
    int val_len_;
    string key_;
    string value_;
    char *buff_;
    unordered_map<string, string> map_;

public:
    inline Answer() {
        map_.reserve(60000);
        buff_ = new char[10];
        fd_ = open(DB_NAME, O_RDWR | O_CREAT, 0600);
        struct stat st;
        fstat(fd_, &st);
        if (st.st_size != 6000000)
            ftruncate(fd_, 6000000);
        mmap_ = (char *) mmap(NULL, 6000000, PROT_WRITE | PROT_READ, MAP_SHARED, fd_, 0);
        madvise(0, 6000000, MADV_SEQUENTIAL | MADV_WILLNEED);
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
                map_[move(key_)] = move(value_);
            }
        }
    }

    inline string get(string key) {
        auto iter = map_.find(move(key));
        if (iter == map_.end()) {
            return "NULL";
        } else {
            return iter->second;
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
        map_[move(key)] = move(value);
    }
};

#endif //KEYVALUESTORE_ANOTHER_IMPL_KEY_VALUE_H
