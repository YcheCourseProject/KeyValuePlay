//
// Created by cheyulin on 8/27/16.
//

#ifndef KEYVALUESTORE_CORRECT_FINAL_VERSION_KEY_VALUE_H
#define KEYVALUESTORE_CORRECT_FINAL_VERSION_KEY_VALUE_H

#include <algorithm>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>

#define INDEX_FILE_NAME "index.meta"
#define DB_NAME "value.db"

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
string empty_str_ = "";

struct key_value_info {
    string key_str_{""};
    string value_str_{""};
    int value_index_{0};
    int value_length_{0};
};

template<size_t slot_num = 50>
class yche_map {
private:
    fstream db_stream_;
    vector<key_value_info> hash_table_;
    char *value_buffer;
    string result_str_;
    size_t cur_cached_value_size_{0};
    size_t max_cached_value_size_{1000};
    size_t max_slot_size_{slot_num};

public:
    inline yche_map() : hash_table_(slot_num) {
        db_stream_.open(DB_NAME, ios::in | ios::out | ios::app | ios::binary);
        value_buffer = new char[1024 * 32];
    }

    inline void resize(int size) {
        hash_table_.resize(size);
        max_slot_size_ = size;
    }

    inline ~yche_map() {
        delete[]value_buffer;
    }

    inline void set_max_cached_value_size(int size) {
        max_cached_value_size_ = size;
    }

    inline string *find(const string &key) {
        auto index = hash_func(key) % max_slot_size_;
        for (; hash_table_[index].key_str_.size() != 0; index = (index + 1) % max_slot_size_) {
            if (hash_table_[index].key_str_ == key) {
                if (hash_table_[index].value_str_.size() > 0)
                    return &hash_table_[index].value_str_;
                else {
                    db_stream_.seekg(hash_table_[index].value_index_, ios::beg);
                    db_stream_.read(value_buffer, hash_table_[index].value_length_);
                    result_str_ = string(value_buffer, 0, hash_table_[index].value_length_);
                    return &result_str_;
                }
            }
        }
        return nullptr;
    }

    inline void insert_or_replace(string &&key, int value_index, int value_length, string &&value = move(empty_str_),
                                  bool is_write = false) {
        if (is_write) {
            db_stream_.seekp(0, ios::end);
            db_stream_ << value << flush;
        }
        auto index = hash_func(key) % max_slot_size_;
        for (; hash_table_[index].key_str_.size() != 0; index = (index + 1) % max_slot_size_) {
            if (hash_table_[index].key_str_ == key) {
                hash_table_[index].value_index_ = value_index;
                hash_table_[index].value_length_ = value_length;
                if (hash_table_[index].value_str_.size() != 0)
                    hash_table_[index].value_str_ = value;
                return;
            }
        }
        hash_table_[index].key_str_ = key;
        hash_table_[index].value_index_ = value_index;
        hash_table_[index].value_length_ = value_length;

        if (cur_cached_value_size_ < max_cached_value_size_) {
            ++cur_cached_value_size_;
            hash_table_[index].value_str_ = value;
        }
    }
};

class Answer {
private:
    yche_map<> map_;
    fstream index_stream_;
    fstream db_stream_;
    int value_index_{0};
    int length_{0};
    int threshold_{0};
    bool is_init_{false};

    inline void read_index_info() {
        char *value_buf_ = new char[32 * 1024];
        index_stream_.open(INDEX_FILE_NAME, ios::in | ios::out | ios::app | ios::binary);
        db_stream_.open(DB_NAME, ios::in | ios::binary);
        string key_str;
        string prefix_sum_index_str;
        string length_str;
        string value_str;
        for (; index_stream_.good();) {
            getline(index_stream_, key_str);
            if (index_stream_.good()) {
                getline(index_stream_, prefix_sum_index_str);
                getline(index_stream_, length_str);
                value_index_ = stoi(prefix_sum_index_str);
                length_ = stoi(length_str);
                init_map();
                if (value_index_ >= threshold_) {
                    db_stream_.seekg(value_index_, ios::beg);
                    db_stream_.read(value_buf_, length_);
                    value_str = string(value_buf_, 0, length_);
                    map_.insert_or_replace(move(key_str), value_index_, length_, move(value_str));
                }
                else
                    map_.insert_or_replace(move(key_str), value_index_, length_);
            }
        }
        value_index_ += length_;
        delete[]value_buf_;
        index_stream_.clear();
    }

    inline void init_map() {
        db_stream_.seekg(0, ios::end);
        int file_size = db_stream_.tellg();
        if (!is_init_) {
            if (length_ <= 160) {
                map_.set_max_cached_value_size(250000);
                map_.resize(50000);
            } else if (length_ <= 3000) {
                map_.set_max_cached_value_size(300000);
                map_.resize(500000);
                threshold_ = file_size + 1;
            }
            else {
                map_.set_max_cached_value_size(12000);
                map_.resize(50000);
                threshold_ = file_size + 1;
            }
            is_init_ = true;
        }
    }

public:
    inline Answer() {
        read_index_info();
    }

    inline string get(string key) {
        auto str_ptr = map_.find(key);
        if (str_ptr == nullptr)
            return "NULL";
        else
            return *str_ptr;
    }

    inline void put(string key, string value) {
        length_ = value.size();
        init_map();
        index_stream_ << key << "\n" << value_index_ << "\n" << length_ << "\n" << flush;
        map_.insert_or_replace(move(key), value_index_, length_, move(value), true);
        value_index_ += length_;
    }
};

#endif //KEYVALUESTORE_CORRECT_FINAL_VERSION_KEY_VALUE_H