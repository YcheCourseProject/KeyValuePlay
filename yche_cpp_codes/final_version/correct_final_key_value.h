//
// Created by cheyulin on 8/23/16.
//
#ifndef KEYVALUESTORE_FINAL_VERSION_KEY_VALUE_STORE_H
#define KEYVALUESTORE_FINAL_VERSION_KEY_VALUE_STORE_H

#include <algorithm>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include "fcntl.h"

constexpr int INT_SIZE = sizeof(int);
#define INDEX_NAME "index.meta"
#define DB_NAME "value.db"
#define unlikely(x) __builtin_expect((x),0)

using namespace std;
string empty_str;
const static string NULL_STR = "NULL";

hash<string> hash_func;

struct key_value_info {
    string key_str_;
    string val_str_;
    int val_index_{0};
    int val_len_{0};
};

class yche_map {
private:
    fstream db_stream_;
    vector<key_value_info> hash_table_;
    char *value_buffer;
    string result_str_;
    int cur_cached_val_size_{0};
    int max_cached_val_size_{0};
    int max_slot_size_{0};

public:
    yche_map() {
        db_stream_.open(DB_NAME, ios::in | ios::binary);
        value_buffer = new char[1024 * 32];
        result_str_.reserve(1024 * 32);
    }

    ~yche_map() {
        delete[]value_buffer;
    }

    void set_max_cached_value_size(int size) {
        max_cached_val_size_ = size;
    }

    void resize(int size) {
        hash_table_.resize(size);
        max_slot_size_ = size;
    }

    string get(const string &key) {
        auto index = hash_func(key) % max_slot_size_;
        for (; hash_table_[index].key_str_.size() != 0; index = (index + 1) % max_slot_size_) {
            if (hash_table_[index].key_str_ == key) {
                if (hash_table_[index].val_str_.size() > 0)
                    return hash_table_[index].val_str_;
                else {
                    db_stream_.seekg(hash_table_[index].val_index_, ios::beg);
                    db_stream_.read(value_buffer, hash_table_[index].val_len_);
                    result_str_.assign(value_buffer, 0, hash_table_[index].val_len_);
                    return result_str_;
                }
            }
        }
        return NULL_STR;
    }

    void put(string &key, int value_index, int value_length, string &value = empty_str) {
        auto index = hash_func(key) % max_slot_size_;
        for (; hash_table_[index].key_str_.size() != 0; index = (index + 1) % max_slot_size_) {
            if (hash_table_[index].key_str_ == key) {
                hash_table_[index].val_index_ = value_index;
                hash_table_[index].val_len_ = value_length;
                if (hash_table_[index].val_str_.size() != 0)
                    hash_table_[index].val_str_ = move(value);
                return;
            }
        }
        hash_table_[index].key_str_ = move(key);
        hash_table_[index].val_index_ = value_index;
        hash_table_[index].val_len_ = value_length;

        if (cur_cached_val_size_ < max_cached_val_size_) {
            ++cur_cached_val_size_;
            hash_table_[index].val_str_ = move(value);
        }
    }
};

class Answer {
private:
    yche_map map_;
    ofstream index_stream_;
    ofstream db_stream_;
    int key_len_{0};
    int val_index_{0};
    int val_len_{0};
    int threshold_{0};
    int file_size_{0};
    bool is_init_{false};
    string key_str_;
    string value_str_;

    void init_map() {
        if (val_len_ <= 160) {
            map_.resize(50000);
            map_.set_max_cached_value_size(250000);
        } else if (val_len_ <= 3000) {
            map_.resize(500000);
            map_.set_max_cached_value_size(800000);
            threshold_ = file_size_ + 1;
        }
        else {
            map_.resize(50000);
            map_.set_max_cached_value_size(11000);
            threshold_ = file_size_ + 1;
        }
        is_init_ = true;
    }

public:
    void get_db_file_size() {
        int fd_ = open(DB_NAME, O_RDONLY | O_CREAT, 0600);
        struct stat st;
        fstat(fd_, &st);
        file_size_ = st.st_size;
    }

    void read_file() {
        char *buf_ = new char[32 * 1024];
        ifstream index_stream(INDEX_NAME, ios::binary);
        ifstream db_stream(DB_NAME, ios::binary);
        for (; index_stream.good();) {
            index_stream.read((char *) &key_len_, INT_SIZE);
            index_stream.read(buf_, key_len_);
            if (index_stream.good()) {
                key_str_.assign(buf_, key_len_);
                index_stream.read((char *) &val_index_, INT_SIZE);
                index_stream.read((char *) &val_len_, INT_SIZE);
                if (unlikely(!is_init_))
                    init_map();
                if (val_index_ >= threshold_) {
                    db_stream.seekg(val_index_, ios::beg);
                    db_stream.read(buf_, val_len_);
                    value_str_.assign(buf_, 0, val_len_);
                    map_.put(key_str_, val_index_, val_len_, value_str_);
                }
                else
                    map_.put(key_str_, val_index_, val_len_);
            }
        }
        val_index_ = val_index_ + val_len_;
        delete[]buf_;
    }

    Answer() {
        ios::sync_with_stdio(false);
        get_db_file_size();
        read_file();
        index_stream_.open(INDEX_NAME, ios::app | ios::binary);
        db_stream_.open(DB_NAME, ios::app | ios::binary);
    }

    string get(string key) {
        return map_.get(key);
    }

    void put(string key, string value) {
        val_len_ = value.size();
        key_len_ = key.size();
        if (unlikely(!is_init_))
            init_map();
        index_stream_.write((const char *) &key_len_, INT_SIZE);
        index_stream_.write(key.c_str(), key_len_);
        index_stream_.write((const char *) &val_index_, INT_SIZE);
        index_stream_.write((const char *) &val_len_, INT_SIZE);
        index_stream_.flush();

        db_stream_.write(value.c_str(), val_len_);
        db_stream_.flush();

        map_.put(key, val_index_, val_len_, value);
        val_index_ += val_len_;
    }
};

#endif //KEYVALUESTORE_FINAL_VERSION_KEY_VALUE_STORE_H