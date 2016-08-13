//
// Created by cheyulin on 8/10/16.
//

#ifndef KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H
#define KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H

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
#define DB_FILE_NUM 1000
#define TUPLE_IN_MEM_THREASHOLD 4000

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
    array<fstream, DB_FILE_NUM> hash_as_name_file_streams_;
    yche_string_string_map<6000> yche_map_;
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
            hash_as_name_file_streams_[i].open(to_string(i), ios::in | ios::out | ios::app | ios::binary);
            fstream &input_file_stream = hash_as_name_file_streams_[i];
            if (input_file_stream.is_open()) {
                string tmp_string;
                for (; input_file_stream.good();) {
                    getline(input_file_stream, tmp_string);
                    if (tmp_string.size() > 0 && tmp_string.substr(tmp_string.size() - 1) == SEPERATOR_END_STRING) {
                        auto my_pair = split(tmp_string);
                        yche_map_.insert_or_replace(my_pair.first, my_pair.second);
                        count++;
                        if (yche_map_.size() > TUPLE_IN_MEM_THREASHOLD - 1)
                            break;
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
        else {
            auto file_hash_index = get_hash_file_name(key);
            fstream &input_file_stream = hash_as_name_file_streams_[file_hash_index];
            if (!input_file_stream.is_open())
                input_file_stream.open(to_string(file_hash_index), ios::in | ios::out | ios::app | ios::binary);
            if (input_file_stream.is_open()) {
                input_file_stream.seekg(0, ios::end);
                size_t buffer_size = input_file_stream.tellg();
                input_file_stream.seekg(0, std::ios::beg);
                char *file_content = new char[buffer_size];
                input_file_stream.read(file_content, buffer_size);
                stringstream str_stream;
                str_stream << file_content;

                stack<string> tmp_string_vec;
                for (; !tmp_string_vec.empty();) {
                    pair<string, string> tmp_pair = split(tmp_string_vec.top());
                    tmp_string_vec.pop();
                    if (tmp_pair.first == key) {
                        return tmp_pair.second;
                    }
                }
                delete[](file_content);
            }
            return "NULL"; //文件不存在，说明该Key不存在，返回NULL
        }
    }

    inline void put(string &&key, string &&value) { //存储KV
        ++count;
        auto file_hash_index = get_hash_file_name(key);
        auto &output_stream = hash_as_name_file_streams_[file_hash_index];
        if (!output_stream.is_open()) {
            output_stream.open(to_string(file_hash_index),
                               ios::in | ios::out | ios::app | ios::binary);
        }
        output_stream << key << SEPERATOR_STRING << value << SEPERATOR_END_STRING
                      << '\n' << flush;
        string *tmp_ptr = yche_map_.find(key);
        if (tmp_ptr != nullptr)
            yche_map_.insert_or_replace(key, value);
        else if (yche_map_.size() < TUPLE_IN_MEM_THREASHOLD) {
            yche_map_.insert_or_replace(key, value);
        }
    }
};

#endif //KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H