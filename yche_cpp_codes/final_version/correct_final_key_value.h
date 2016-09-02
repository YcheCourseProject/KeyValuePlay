//
// Created by cheyulin on 8/23/16.
//
#ifndef KEYVALUESTORE_FINAL_VERSION_KEY_VALUE_STORE_H
#define KEYVALUESTORE_FINAL_VERSION_KEY_VALUE_STORE_H

#include <algorithm>
#include <fstream>

#define INDEX_NAME "index.meta"
#define DB_NAME "value.db"
#define unlikely(x) __builtin_expect((x),0)

using namespace std;
string empty_str;

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

    string *find(const string &key) {
        auto index = hash_func(key) % max_slot_size_;
        for (; hash_table_[index].key_str_.size() != 0; index = (index + 1) % max_slot_size_) {
            if (hash_table_[index].key_str_ == key) {
                if (hash_table_[index].val_str_.size() > 0)
                    return &hash_table_[index].val_str_;
                else {
                    db_stream_.seekg(hash_table_[index].val_index_, ios::beg);
                    db_stream_.read(value_buffer, hash_table_[index].val_len_);
                    result_str_.assign(value_buffer, 0, hash_table_[index].val_len_);
                    return &result_str_;
                }
            }
        }
        return nullptr;
    }

    void insert_or_replace(string &key, int value_index, int value_length, string &value = empty_str) {
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
    fstream index_stream_;
    fstream db_stream_;
    int val_index_{0};
    int val_len_{0};
    int key_len_{0};
    int threshold_{0};
    bool is_init_{false};

    void init_map() {
        db_stream_.seekg(0, ios::end);
        int file_size = db_stream_.tellg();
        if (val_len_ <= 160) {
            map_.resize(50000);
            map_.set_max_cached_value_size(250000);
        } else if (val_len_ <= 3000) {
            map_.resize(500000);
            map_.set_max_cached_value_size(800000);
            threshold_ = file_size + 1;
        }
        else {
            map_.resize(50000);
            map_.set_max_cached_value_size(11000);
            threshold_ = file_size + 1;
        }
        is_init_ = true;
    }

public:
    Answer() {
        char *val_buf_ = new char[32 * 1024];
        index_stream_.open(INDEX_NAME, ios::in | ios::out | ios::app | ios::binary);
        db_stream_.open(DB_NAME, ios::in | ios::out | ios::app | ios::binary);
        string key_str;
        string prefix_sum_index_str;
        string length_str;
        string value_str;
        for (; index_stream_.good();) {
            getline(index_stream_, key_str);
            if (index_stream_.good()) {
                getline(index_stream_, prefix_sum_index_str);
                getline(index_stream_, length_str);
                val_index_ = stoi(prefix_sum_index_str);
                val_len_ = stoi(length_str);
                if (unlikely(!is_init_))
                    init_map();
                if (val_index_ >= threshold_) {
                    db_stream_.seekg(val_index_, ios::beg);
                    db_stream_.read(val_buf_, val_len_);
                    value_str.assign(val_buf_, 0, val_len_);
                    map_.insert_or_replace(key_str, val_index_, val_len_, value_str);
                }
                else
                    map_.insert_or_replace(key_str, val_index_, val_len_);
            }
        }
        val_index_ = val_index_ + val_len_;
        delete[]val_buf_;
        index_stream_.clear();
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
        val_len_ = value.size();
        key_len_ = key.size();
        if (unlikely(!is_init_))
            init_map();
        index_stream_ << key << "\n" << val_index_ << "\n" << val_len_ << "\n" << flush;
        db_stream_ << value << flush;
        map_.insert_or_replace(key, val_index_, val_len_, value);
        val_index_ += val_len_;
    }
};

#endif //KEYVALUESTORE_FINAL_VERSION_KEY_VALUE_STORE_H