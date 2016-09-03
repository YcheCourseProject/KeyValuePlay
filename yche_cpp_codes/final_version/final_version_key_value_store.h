//
// Created by cheyulin on 8/23/16.
//

#ifndef KEYVALUESTORE_FINAL_VERSION_KEY_VALUE_STORE_H
#define KEYVALUESTORE_FINAL_VERSION_KEY_VALUE_STORE_H

#include <algorithm>
#include <string>
#include <queue>
#include <fstream>

#define INDEX_NAME "index.meta"
#define DB_NAME "value.db"

using namespace std;

hash<string> hash_func;

struct key_value_info {
    string key_str_{""};
    string val_str_{""};
    int val_index_{0};
    int val_len_{0};
    int insert_count_{0};
};

template<size_t slot_num = 100>
class yche_map {
private:
    fstream val_stream_;

    vector<key_value_info> hash_table_;
    queue<int> slot_index_queue_;
    char *val_buffer;
    string result_str_;

    size_t max_slot_size_{slot_num};
    size_t cur_cached_memory_size_{0};
    size_t max_cached_memory_size_{1000};

public:
    inline yche_map() : hash_table_(slot_num) {
        val_stream_.open(DB_NAME, ios::in | ios::out | ios::app | ios::binary);
        val_buffer = new char[1024 * 32];
    }

    inline void resize(int size) {
        hash_table_.resize(size);
        max_slot_size_ = size;
    }

    inline ~yche_map() {
        delete[]val_buffer;
    }

    inline void set_max_cached_memory_size(int size) {
        max_cached_memory_size_ = size;
    }

    inline void pop_until_below_threshold() {
        for (; cur_cached_memory_size_ > max_cached_memory_size_;) {
            auto index = slot_index_queue_.front();
            auto &cur_key_val_info = hash_table_[index];
            --cur_key_val_info.insert_count_;
            if (cur_key_val_info.insert_count_ == 0) {
                cur_cached_memory_size_ -= cur_key_val_info.val_len_;
                cur_key_val_info.val_str_.resize(0);
            }
            slot_index_queue_.pop();
        }
    }

    inline string *get(const string &key) {
        auto index = hash_func(key) % max_slot_size_;
        for (; hash_table_[index].key_str_.size() != 0; index = (index + 1) % max_slot_size_) {
            if (hash_table_[index].key_str_ == key) {
                if (hash_table_[index].val_str_.size() > 0)
                    return &hash_table_[index].val_str_;
                else {
                    val_stream_.seekg(hash_table_[index].val_index_, ios::beg);
                    val_stream_.read(val_buffer, hash_table_[index].val_len_);
                    result_str_ = string(val_buffer, 0, hash_table_[index].val_len_);
                    return &result_str_;
                }
            }
        }
        return nullptr;
    }

    inline void insert_or_replace(const string &key, int value_index, int value_length, const string value = "",
                                  bool is_write = false) {
        if (is_write) {
            val_stream_.seekp(0, ios::end);
            val_stream_ << value << flush;
        }

        auto index = hash_func(key) % max_slot_size_;
        bool is_key_already_in = false;
        for (; hash_table_[index].key_str_.size() != 0; index = (index + 1) % max_slot_size_) {
            if (hash_table_[index].key_str_ == key) {
                is_key_already_in = true;
                break;
            }
        }

        if (value.size() > 0) {
            cur_cached_memory_size_ += value.size() - hash_table_[index].val_len_;
            pop_until_below_threshold();
            hash_table_[index].val_str_ = move(value);
            slot_index_queue_.push(index);
            ++hash_table_[index].insert_count_;
        }

        if (!is_key_already_in)
            hash_table_[index].key_str_ = move(key);
        hash_table_[index].val_index_ = value_index;
        hash_table_[index].val_len_ = value_length;

    }
};

class Answer {
private:
    yche_map<> map_;
    fstream index_stream_;
    fstream val_stream_;
    int val_index_{0};
    int val_len_{0};
    int threshold_{0};
    bool is_init_{false};

    inline void read_index_info() {
        char *value_buf_ = new char[32 * 1024];
        index_stream_.open(INDEX_NAME, ios::in | ios::out | ios::app | ios::binary);
        val_stream_.open(DB_NAME, ios::in | ios::binary);
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
                init_map();
                if (val_index_ >= threshold_) {
                    val_stream_.seekg(val_index_, ios::beg);
                    val_stream_.read(value_buf_, val_len_);
                    value_str = string(value_buf_, 0, val_len_);
                    map_.insert_or_replace(key_str, val_index_, val_len_, value_str);
                }
                else
                    map_.insert_or_replace(key_str, val_index_, val_len_);
            }
        }
        val_index_ += val_len_;
        delete[]value_buf_;
        index_stream_.clear();
    }

    inline void init_map() {
        val_stream_.seekg(0, ios::end);
        int file_size = val_stream_.tellg();
        if (!is_init_) {
            if (val_len_ <= 160) {
                map_.set_max_cached_memory_size(20000000);
                map_.resize(60000);
            } else if (val_len_ <= 3000) {
                map_.set_max_cached_memory_size(15000000);
                map_.resize(600000);
                threshold_ = file_size + 1;
            }
            else {
                map_.set_max_cached_memory_size(50000000);
                map_.resize(60000);
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
        auto str_ptr = map_.get(key);
        if (str_ptr == nullptr)
            return "NULL";
        else
            return *str_ptr;
    }

    inline void put(string key, string value) {
        val_len_ = value.size();
        init_map();
        index_stream_ << key << "\n" << val_index_ << "\n" << val_len_ << "\n" << flush;
        map_.insert_or_replace(key, val_index_, val_len_, value, true);
        val_index_ += val_len_;
    }
};

#endif //KEYVALUESTORE_FINAL_VERSION_KEY_VALUE_STORE_H