//
// Created by cheyulin on 8/23/16.
//

#ifndef KEYVALUESTORE_FINAL_VERSION_KEY_VALUE_STORE_H
#define KEYVALUESTORE_FINAL_VERSION_KEY_VALUE_STORE_H

#include <unordered_map>
#include <algorithm>
#include <string>
#include <cstring>
#include <fstream>

#define INDEX_FILE_NAME "index.meta"
#define DB_NAME "value.db"

using namespace std;

hash<string> hash_func;

template<size_t slot_num = 900000>
class yche_map {
private:
    fstream db_stream_;

    vector<pair<string, pair<int, int>>> my_hash_table_;
    vector<string> value_table_;
    char *value_buffer;

    size_t cached_value_size_{0};
    size_t current_size_{0};
    size_t slot_max_size_{slot_num};
    size_t max_cached_value_size_{150000};
    string result_string_;

public:
    inline yche_map() : my_hash_table_(slot_num), value_table_(slot_num) {
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
        auto index = hash_func(key) % slot_max_size_;
        for (; my_hash_table_[index].first.size() != 0; index = (index + 1) % slot_max_size_) {
            if (my_hash_table_[index].first == key) {
                if (value_table_[index].size() > 0)
                    return &value_table_[index];
                else {
                    db_stream_.seekg(my_hash_table_[index].second.first, ios::beg);
                    db_stream_.read(value_buffer, my_hash_table_[index].second.second);
                    result_string_ = string(value_buffer, 0, my_hash_table_[index].second.second);
                    return &result_string_;
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
        auto index = hash_func(key) % slot_max_size_;
        for (; my_hash_table_[index].first.size() != 0; index = (index + 1) % slot_max_size_) {
            if (my_hash_table_[index].first == key) {
                my_hash_table_[index].second = make_pair(value_index, value_length);
                value_table_[index] = value;
                return;
            }
        }
        my_hash_table_[index].first = key;
        my_hash_table_[index].second = make_pair(value_index, value_length);

        if (cached_value_size_ < max_cached_value_size_) {
            ++cached_value_size_;
            value_table_[index] = value;
        }
    }
};

class Answer {
private:
    yche_map<> yche_map_;
    fstream key_index_stream_;
    int prefix_sum_index_{0};
    int length_{0};
    bool is_init_{false};

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
                yche_map_.insert_or_replace(key_str, prefix_sum_index_, length_);
            }
        }
        prefix_sum_index_ = prefix_sum_index_ + length_;
        key_index_stream_.clear();
    }

public:
    Answer() {
        key_index_stream_.open(INDEX_FILE_NAME, ios::in | ios::out | ios::app | ios::binary);
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
        if (!is_init_) {
            if (length_ < 5000)
                yche_map_.set_max_cached_value_size(150000);
            else
                yche_map_.set_max_cached_value_size(1000);
            is_init_ = true;
        }
        key_index_stream_ << key << "\n";
        key_index_stream_ << prefix_sum_index_ << "\n";
        key_index_stream_ << length_ << "\n" << flush;

        yche_map_.insert_or_replace(key, prefix_sum_index_, length_, value, true);
        prefix_sum_index_ += length_;
    }
};


#endif //KEYVALUESTORE_FINAL_VERSION_KEY_VALUE_STORE_H
