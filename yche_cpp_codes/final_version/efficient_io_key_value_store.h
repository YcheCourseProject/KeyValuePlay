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

hash<string> hash_func;

template<typename T, size_t slot_num = 200000>
class yche_map {
private:
    vector<pair<string, T>> my_hash_table_;
    size_t current_size_{0};
    size_t slot_max_size_{slot_num};

public:
    inline yche_map() : my_hash_table_(slot_num) {}

    inline void reserve(int size) {
        my_hash_table_.resize(size);
    }

    inline size_t size() {
        return current_size_;
    }

    inline T *find(const string &key) {
        auto index = hash_func(key) % slot_max_size_;
        for (; my_hash_table_[index].first.size() != 0; index = (index + 1) % slot_max_size_) {
            if (my_hash_table_[index].first == key) {
                return &my_hash_table_[index].second;
            }
        }
        return nullptr;
    }

    inline void insert_or_replace(const string &key, const T &value) {
        auto index = hash_func(key) % slot_max_size_;
        for (; my_hash_table_[index].first.size() != 0; index = (index + 1) % slot_max_size_) {
            if (my_hash_table_[index].first == key) {
                my_hash_table_[index].second = value;
                return;
            }
        }
        ++current_size_;
        my_hash_table_[index].first = key;
        my_hash_table_[index].second = value;
    }
};

class Answer {
private:
    yche_map<pair<int, int>> key_index_info_map_;
    fstream key_index_stream_;
    fstream db_stream_;

    int prefix_sum_index_{0};
    int length_{0};

    char *value_buffer;
    bool is_first_in_{true};

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
                    init_map_info(length_);
                    is_first_in_ = false;
                }
                key_index_info_map_.insert_or_replace(key_str, make_pair(prefix_sum_index_, length_));
            }
        }
        prefix_sum_index_ = prefix_sum_index_ + length_;
        key_index_stream_.clear();
    }

    inline void init_map_info(int length) {
        if (length < 500) {
            key_index_info_map_.reserve(60000);
        } else if (length < 5000) {
            key_index_info_map_.reserve(1500000);
        } else {
            key_index_info_map_.reserve(60000);
        }
    }

public:
    Answer() {
        value_buffer = new char[1024 * 32];
        key_index_stream_.open(INDEX_FILE_NAME, ios::in | ios::out | ios::app | ios::binary);
        db_stream_.open(DB_NAME, ios::in | ios::out | ios::app | ios::binary);
        read_index_info();
    }

    virtual ~Answer() {
        delete[] value_buffer;
    }

    inline string get(string key) {
        if (key_index_info_map_.find(key) == nullptr) {
            return "NULL";
        }
        else {
            auto *index_pair = key_index_info_map_.find(key);
            db_stream_.seekg(index_pair->first, ios::beg);
            db_stream_.read(value_buffer, index_pair->second);
            return string(value_buffer, 0, index_pair->second);
        }
    }

    inline void put(string key, string value) {
        length_ = value.size();
        if (is_first_in_) {
            init_map_info(length_);
            is_first_in_ = false;
        }
        key_index_stream_ << key << "\n";
        key_index_stream_ << prefix_sum_index_ << "\n";
        key_index_stream_ << length_ << "\n" << flush;

        db_stream_.seekp(0, ios::end);
        db_stream_ << value << flush;

        key_index_info_map_.insert_or_replace(key, make_pair(prefix_sum_index_, length_));
        prefix_sum_index_ += length_;

    }
};

#endif //KEYVALUESTORE_EFFICIENT_IO_KEY_VALUE_STORE_H