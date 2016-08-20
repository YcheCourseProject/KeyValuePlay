//
// Created by cheyulin on 8/20/16.
//

#ifndef KEYVALUESTORE_EFFICIENT_IO_KEY_VALUE_STORE_H
#define KEYVALUESTORE_EFFICIENT_IO_KEY_VALUE_STORE_H

#include <unordered_map>
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>

#define INDEX_FILE_NAME "index.meta"
#define DB_NAME "value.db"

using namespace std;

class Answer {
private:
    unordered_map<string, pair<uint32_t, uint32_t>> key_index_info_map_;
    fstream key_index_stream_;
    fstream db_stream_;
    uint32_t prefix_sum_index_{0};
    uint32_t length_{0};
    char *transfer_buffer;
    char *value_buffer;

    void serialize_integer(char *buf, uint32_t val) {
        memcpy(buf, &val, 4);
    }

    int32_t parse_integer(const char *buf) {
        uint32_t val;
        memcpy(&val, buf, 4);
        return val;
    }

    inline void read_index_info() {
        string key_str;
        string prefix_sum_index_str;
        string length_str;
        for (; key_index_stream_.good();) {
            getline(key_index_stream_, key_str);
            if (key_index_stream_.good()) {
                getline(key_index_stream_, prefix_sum_index_str);
                getline(key_index_stream_, length_str);
                key_index_info_map_[key_str] = make_pair(parse_integer(prefix_sum_index_str.c_str()),
                                                         parse_integer(length_str.c_str()));
            }
        }
        key_index_stream_.clear();
    }

public:
    Answer() {
        transfer_buffer = new char[4];
        value_buffer = new char[1024 * 1024];
        key_index_stream_.open(INDEX_FILE_NAME, ios::in | ios::out | ios::app | ios::binary);
        db_stream_.open(DB_NAME, ios::in | ios::out | ios::app | ios::binary);
        read_index_info();
    }

    virtual ~Answer() {
        delete[]transfer_buffer;
        delete[] value_buffer;
    }

    inline string get(string key) {
        if (key_index_info_map_.find(key) == key_index_info_map_.end()) {
            return "NULL";
        }
        else {
            db_stream_.seekg(key_index_info_map_[key].first, ios::beg);
            db_stream_.read(value_buffer, key_index_info_map_[key].second);
            return string(value_buffer, 0, key_index_info_map_[key].second);
        }
    }

    inline void put(string key, string value) {
        auto value_size = value.size();
        key_index_stream_ << key << "\n";
        serialize_integer(transfer_buffer, prefix_sum_index_);
        key_index_stream_ << string(transfer_buffer, 0, 4) << "\n";
        serialize_integer(transfer_buffer, value_size);
        key_index_stream_ << string(transfer_buffer, 0, 4) << "\n" << flush;

        key_index_info_map_[key] = make_pair(prefix_sum_index_, value_size);
        prefix_sum_index_ += value_size;

        db_stream_.seekp(0, ios::end);
        db_stream_ << value << flush;
    }
};


#endif //KEYVALUESTORE_EFFICIENT_IO_KEY_VALUE_STORE_H
