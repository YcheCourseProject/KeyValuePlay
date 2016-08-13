//
// Created by cheyulin on 8/10/16.
//

#ifndef KeyValueStore_KEY_VALUE_STORE_H
#define KeyValueStore_KEY_VALUE_STORE_H

#include <string>
#include <fstream>
#include <sstream>
#include <cstddef>
#include <algorithm>
#include <stack>
#include <array>

using namespace std;

#define FILE_NAME "tuple_transaction.db"
#define SEPERATOR_STRING ","
#define SEPERATOR_END_STRING ";"
#define HASH_FUNC(x) str_hash_func_basic(x)
#define DB_FILE_NUM 10
#define TUPLE_IN_MEM_THRESHOLD 1

std::hash<string> str_hash_func_basic;

template<size_t slot_num = 90000>
class yche_string_string_map {
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

inline int get_hash_file_name(const string &to_persistent_string) {
    return HASH_FUNC(to_persistent_string) % DB_FILE_NUM;
}

class Answer {
private:
    yche_string_string_map<1> yche_map_;
    size_t count{0};

    inline pair<string, string> split(const string &str) {
        auto iter_begin = str.begin();
        auto iter_end = str.end();
        auto iter_middle = find(iter_begin, iter_end, ',');
        return make_pair(string(iter_begin, iter_middle),
                         string(iter_middle + 1, iter_end - 1));
    }

public:
    Answer() {
        for (auto i = 0; i < DB_FILE_NUM; ++i) {
            fstream input_stream;
            input_stream.open(to_string(i), ios::in | ios::binary);
            fstream &input_file_stream = input_stream;
            if (input_file_stream.is_open()) {
                string tmp_string;
                for (; input_file_stream.good() && yche_map_.size() < TUPLE_IN_MEM_THRESHOLD;) {
                    getline(input_file_stream, tmp_string);
                    if (tmp_string.size() > 0 && tmp_string.substr(tmp_string.size() - 1) == SEPERATOR_END_STRING) {
                        auto my_pair = split(tmp_string);
                        yche_map_.insert_or_replace(my_pair.first, my_pair.second);
                    }
                }
            }
        }
    }

    inline string get(string &&key) { //读取KV
        auto result = yche_map_.find(key);
        if (result != nullptr) {
            return *result;
        }
        else if (yche_map_.size() < TUPLE_IN_MEM_THRESHOLD) {
            return "NULL";
        }
        else {
            auto file_hash_index = get_hash_file_name(key);
            fstream input_stream(to_string(file_hash_index), ios::in | ios::binary);
            fstream &input_file_stream = input_stream;
            if (input_file_stream.is_open()) {
                string tmp_string;
                string *result_ptr = nullptr;
                for (; input_file_stream.good();) {
                    getline(input_file_stream, tmp_string);
                    if (tmp_string.size() > 0 && tmp_string.substr(tmp_string.size() - 1) == SEPERATOR_END_STRING) {
                        auto my_pair = split(tmp_string);
                        if (my_pair.first == key) {
                            result_ptr = &my_pair.second;
                        }
                    }
                }
                if (result_ptr != nullptr) {
                    return *result_ptr;
                }
            }
            return "NULL"; //文件不存在，说明该Key不存在，返回NULL
        }
    }

    inline void put(string &&key, string &&value) { //存储KV
        ++count;
        auto file_hash_index = get_hash_file_name(key);
        fstream my_output_stream(to_string(file_hash_index), ios::in | ios::out | ios::app | ios::binary);
        auto &output_stream = my_output_stream;
        output_stream << key << SEPERATOR_STRING << value << SEPERATOR_END_STRING
                      << '\n' << flush;
        string *tmp_ptr = yche_map_.find(key);
        if (tmp_ptr != nullptr)
            yche_map_.insert_or_replace(key, value);
        else if (yche_map_.size() < TUPLE_IN_MEM_THRESHOLD) {
            yche_map_.insert_or_replace(key, value);
        }
    }
};

#endif //INMEMORYKEYVALUESTOREWITHPERSISTENCE_KEY_VALUE_STORE_H