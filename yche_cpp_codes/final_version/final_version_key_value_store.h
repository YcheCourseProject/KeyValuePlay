//
// Created by cheyulin on 8/23/16.
//

#ifndef KEYVALUESTORE_FINAL_VERSION_KEY_VALUE_STORE_H
#define KEYVALUESTORE_FINAL_VERSION_KEY_VALUE_STORE_H

#include <unordered_map>
#include <algorithm>
#include <string>
#include <fstream>

#define INDEX_FILE_NAME "index.meta"
#define DB_NAME "value.db"

using namespace std;

hash<string> hash_func;

template<size_t slot_num = 900000>
class yche_map {
private:
    fstream db_stream_;

    vector<pair<string, pair<int, int>>> hash_table_;
    vector<string> value_table_;
    char *value_buffer;
    string result_str_;

    size_t cur_cached_value_size_{0};
    size_t max_cached_value_size_{1000};
    size_t max_slot_size_{slot_num};

public:
    inline yche_map() : hash_table_(slot_num), value_table_(slot_num) {
        db_stream_.open(DB_NAME, ios::in | ios::out | ios::app | ios::binary);
        value_buffer = new char[1024 * 32];
    }

    inline ~yche_map() {
        delete[]value_buffer;
    }

    inline void set_max_cached_value_size(int size) {
        max_cached_value_size_ = size;
    }

    inline string *find(const string &key) {
        auto index = hash_func(key) % max_slot_size_;
        for (; hash_table_[index].first.size() != 0; index = (index + 1) % max_slot_size_) {
            if (hash_table_[index].first == key) {
                if (value_table_[index].size() > 0)
                    return &value_table_[index];
                else {
                    db_stream_.seekg(hash_table_[index].second.first, ios::beg);
                    db_stream_.read(value_buffer, hash_table_[index].second.second);
                    result_str_ = string(value_buffer, 0, hash_table_[index].second.second);
                    return &result_str_;
                }
            }
        }
        return nullptr;
    }

    inline void insert_or_replace(const string &key, int value_index, int value_length, const string value = "",
                                  bool is_write = false) {
        if (is_write) {
            db_stream_.seekp(0, ios::end);
            db_stream_ << value << flush;
        }
        auto index = hash_func(key) % max_slot_size_;
        for (; hash_table_[index].first.size() != 0; index = (index + 1) % max_slot_size_) {
            if (hash_table_[index].first == key) {
                hash_table_[index].second = make_pair(value_index, value_length);
                if (value_table_[index].size() != 0)
                    value_table_[index] = value;
                return;
            }
        }
        hash_table_[index].first = key;
        hash_table_[index].second = make_pair(value_index, value_length);

        if (cur_cached_value_size_ < max_cached_value_size_) {
            ++cur_cached_value_size_;
            value_table_[index] = value;
        }
    }
};

class Answer {
private:
    yche_map<> yche_map_;
    fstream index_stream_;
    int value_index_{0};
    int length_{0};
    bool is_init_{false};

    inline void read_index_info() {
        index_stream_.open(INDEX_FILE_NAME, ios::in | ios::out | ios::app | ios::binary);
        string key_str;
        string prefix_sum_index_str;
        string length_str;
        for (; index_stream_.good();) {
            getline(index_stream_, key_str);
            if (index_stream_.good()) {
                getline(index_stream_, prefix_sum_index_str);
                getline(index_stream_, length_str);
                value_index_ = stoi(prefix_sum_index_str);
                length_ = stoi(length_str);
                int_map();
                yche_map_.insert_or_replace(key_str, value_index_, length_);
            }
        }
        value_index_ = value_index_ + length_;
        index_stream_.clear();
    }

    inline void int_map() {
        if (!is_init_) {
            if (length_ <= 3000)
                yche_map_.set_max_cached_value_size(150000);
            else
                yche_map_.set_max_cached_value_size(4500);
            is_init_ = true;
        }
    }

public:
    Answer() {
        read_index_info();
    }

    inline string get(string key) {
        auto str_ptr = yche_map_.find(key);
        if (str_ptr == nullptr) {
            return "NULL";
        }
        else {
            return *str_ptr;
        }
    }

    inline void put(string key, string value) {
        length_ = value.size();
        int_map();
        index_stream_ << key << "\n" << value_index_ << "\n" << length_ << "\n" << flush;

        yche_map_.insert_or_replace(key, value_index_, length_, value, true);
        value_index_ += length_;
    }
};

#endif //KEYVALUESTORE_FINAL_VERSION_KEY_VALUE_STORE_H
