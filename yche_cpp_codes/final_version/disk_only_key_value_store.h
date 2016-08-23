//
// Created by cheyulin on 8/22/16.
//

#ifndef KEYVALUESTORE_DISK_ONLY_KEY_VALUE_STORE_H
#define KEYVALUESTORE_DISK_ONLY_KEY_VALUE_STORE_H

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
                key_index_info_map_[key_str] = make_pair(prefix_sum_index_, length_);
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
        if (key_index_info_map_.find(key) == key_index_info_map_.end()) {
            return "NULL";
        }
        else {
            auto &index_pair = key_index_info_map_[key];
            db_stream_.seekg(index_pair.first, ios::beg);
            db_stream_.read(value_buffer, index_pair.second);
            return string(value_buffer, 0, index_pair.second);
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

        key_index_info_map_[key] = make_pair(prefix_sum_index_, length_);
        prefix_sum_index_ += length_;

    }
};

#endif //KEYVALUESTORE_DISK_ONLY_KEY_VALUE_STORE_H