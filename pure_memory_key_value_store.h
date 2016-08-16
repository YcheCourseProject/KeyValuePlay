//
// Created by cheyulin on 8/10/16.
//

#ifndef KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H
#define KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>

using namespace std;

#define HASH_FUNC(x) str_hash_func_basic(x)
constexpr char *FILE_NAME = "tuple_transaction.db";
constexpr char *SEPERATOR_STRING = ",";
constexpr int DEFAULT_HASH_TABLE_SLOT_SIZE = 80000;

std::hash<string> str_hash_func_basic;

template<size_t slot_num = DEFAULT_HASH_TABLE_SLOT_SIZE>
class yche_string_string_map {
private:
    vector<pair<string, string>> my_hash_table_;
    size_t current_size_{0};
    size_t slot_max_size_{slot_num};

    inline void rebuild() {
        vector<pair<string, string>> my_hash_table_building;
        slot_max_size_ *= 2;
        my_hash_table_building.resize(slot_max_size_);
        for (auto previous_index = 0; previous_index < slot_max_size_ / 2; ++previous_index) {
            if (my_hash_table_[previous_index].first.size() > 0) {
                auto new_index = HASH_FUNC(my_hash_table_[previous_index].first) % slot_max_size_;
                for (; my_hash_table_building[new_index].first.size() != 0;
                       new_index = (++new_index) % slot_max_size_) {
                }
                my_hash_table_building[new_index] = std::move(my_hash_table_[previous_index]);
            }

        }
        my_hash_table_ = std::move(my_hash_table_building);
    }


public:
    yche_string_string_map() : my_hash_table_(slot_num) {}

    inline void resize_at_begin(int size) {
        my_hash_table_.resize(size);
    }

    inline size_t size() {
        return current_size_;
    }

    inline string *find(const string &key) {
        auto index = HASH_FUNC(key) % slot_max_size_;
        //linear probing
        for (; my_hash_table_[index].first.size() != 0; index = (++index) % slot_max_size_) {
            if (my_hash_table_[index].first == key) {
                return &my_hash_table_[index].second;
            }
        }
        return nullptr;
    }

    inline void insert_or_replace(const string &key, const string &value) {
        auto index = HASH_FUNC(key) % slot_max_size_;
        for (; my_hash_table_[index].first.size() != 0; index = (++index) % slot_max_size_) {
            if (my_hash_table_[index].first == key) {
                my_hash_table_[index].second = value;
                return;
            }
        }
        ++current_size_;
        my_hash_table_[index].first = key;
        my_hash_table_[index].second = value;
        if (current_size_ / slot_max_size_ > 0.8) {
            rebuild();
        }
    }
};

class Answer {
private:
    yche_string_string_map<90000> yche_map_;
    fstream db_file_stream_;
    size_t count{0};

    inline pair<string, string> split(const string &str) {
        auto iter_begin = str.begin();
        auto iter_end = str.end();
        auto iter_middle = find(iter_begin, iter_end, ',');
        return make_pair(string(iter_begin, iter_middle),
                         string(iter_middle + 1, iter_end));
    }

public:
    Answer() {
        fstream input_file_stream{FILE_NAME, ios::in | ios::out | ios::app | ios::binary};
        string tmp_string;
        for (; input_file_stream.good();) {
            getline(input_file_stream, tmp_string);
            if (input_file_stream.good()) {
                auto my_pair = split(tmp_string);
                yche_map_.insert_or_replace(my_pair.first, my_pair.second);
            }
        }

//        db_file_stream_.clear();
        //invalidate due to eof flag
        db_file_stream_.open(FILE_NAME, ios::out | ios::app | ios::binary);
    }

    inline string get(string &&key) { //读取KV
        auto result = yche_map_.find(key);
        if (result != nullptr) {
            return *result;
        }
        else {
            return "NULL"; //文件不存在，说明该Key不存在，返回NULL
        }
    }

    inline void put(string &&key, string &&value) { //存储KV
        ++count;
        db_file_stream_ << key << SEPERATOR_STRING << value << '\n';
        if ((count % 13 == 1 || count % 13 == 3 || count % 13 == 11)) {
            db_file_stream_ << flush;
        }
        yche_map_.insert_or_replace(key, value);
    }
};

#endif //KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H