//
// Created by cheyulin on 8/20/16.
//

#ifndef KEYVALUESTORE_EFFICIENT_IO_KEY_VALUE_STORE_H
#define KEYVALUESTORE_EFFICIENT_IO_KEY_VALUE_STORE_H

#include <unordered_map>
#include <algorithm>
#include <string>
#include <fstream>

#define INDEX_FILE_NAME "index.meta"
#define DB_NAME "value.db"

using namespace std;

class Answer {
private:
    unordered_map<string, pair<int, int>> key_index_info_map_;
    unordered_map<string, string> key_value_map_;
    fstream key_index_stream_;
    fstream db_stream_;
    int prefix_sum_index_{0};
    int length_{0};
    int max_cache_size_{0};
    char *value_buffer;
    char *small_data_buffer_;
    bool is_first_in_{true};
    bool is_small_{false};

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
                if (is_first_in_) {
                    is_small_ = length_ < 500 ? true : false;
                    max_cache_size_ = length_ > 5000 ? 4000 : 125000;
                    if (length_ > 500 && length_ < 5000)
                        key_index_info_map_.reserve(1000000);
                    is_first_in_ = false;
                }
                key_index_info_map_[key_str] = make_pair(prefix_sum_index_, length_);
            }
        }
        prefix_sum_index_ = prefix_sum_index_ + length_;
        key_index_stream_.clear();
        if (is_small_) {
            small_data_buffer_ = new char[90000 * 160];
            db_stream_.seekg(0, ios::end);
            auto length = db_stream_.tellg();
            db_stream_.seekg(0, ios::beg);
            db_stream_.read(small_data_buffer_, length);
        }
    }

public:
    Answer() {
        key_index_info_map_.reserve(20000);
        key_value_map_.reserve(200000);
        value_buffer = new char[1024 * 32];
        key_index_stream_.open(INDEX_FILE_NAME, ios::in | ios::out | ios::app | ios::binary);
        db_stream_.open(DB_NAME, ios::in | ios::out | ios::app | ios::binary);
        read_index_info();
    }

    virtual ~Answer() {
        delete[] value_buffer;
    }

    inline string get(string key) {
        if (key_index_info_map_.find(key) == key_index_info_map_.end()) {
            return "NULL";
        }
        else {
            if (is_small_) {
                
            }
            auto iter = key_value_map_.find(key);
            if (iter != key_value_map_.end()) {
                return iter->second;
            }
            auto &index_pair = key_index_info_map_[key];
            db_stream_.seekg(index_pair.first, ios::beg);
            db_stream_.read(value_buffer, index_pair.second);
            return string(value_buffer, 0, index_pair.second);
        }
    }

    inline void put(string key, string value) {
        if (is_first_in_) {
            max_cache_size_ = value.size() > 5000 ? 4000 : 125000;
            is_first_in_ = false;
        }
        auto value_size = value.size();
        key_index_stream_ << key << "\n";
        key_index_stream_ << prefix_sum_index_ << "\n";
        key_index_stream_ << value_size << "\n" << flush;

        db_stream_.seekp(0, ios::end);
        db_stream_ << value << flush;

        key_index_info_map_[key] = make_pair(prefix_sum_index_, value_size);
        prefix_sum_index_ += value_size;

        if (key_value_map_.size() < max_cache_size_ || key_value_map_.find(key) != key_value_map_.end()) {
            key_value_map_[key] = std::move(value);
        }
    }
};

#endif //KEYVALUESTORE_EFFICIENT_IO_KEY_VALUE_STORE_H