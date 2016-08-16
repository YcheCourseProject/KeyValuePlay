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
#include <array>
#include <memory>

using namespace std;

#define SEPERATOR_STRING ","
#define SEPERATOR_END_STRING ";"
#define HASH_FUNC(x) str_hash_func_basic(x)
#define DB_FILE_NUM 1000

template<typename _Tp, typename... _Args>
inline unique_ptr<_Tp> make_unique(_Args &&... __args) {
    return unique_ptr<_Tp>(new _Tp(std::forward<_Args>(__args)...));
}

std::hash<string> str_hash_func_basic;

inline size_t get_hash_file_index(const string &to_persistent_string) {
    return HASH_FUNC(to_persistent_string) % DB_FILE_NUM;
}

inline pair<string, string> split(const string &str) {
    auto iter_begin = str.begin();
    auto iter_end = str.end();
    auto iter_middle = find(iter_begin, iter_end, ',');
    return make_pair(string(iter_begin, iter_middle),
                     string(iter_middle + 1, iter_end));
}

class Answer {
private:
    array<unique_ptr<fstream>, DB_FILE_NUM> db_file_array_;

public:
    Answer() {
        for (auto i = 0; i < DB_FILE_NUM; i++) {
            db_file_array_[i] = make_unique<fstream>(to_string(i), ios::in | ios::out | ios::app | ios::binary);
        }
    }

    inline string get(string &&key) { //读取KV
        size_t file_hash_index = get_hash_file_index(key);
        auto &input_file_stream = *db_file_array_[file_hash_index];
        input_file_stream.seekg(0, ios::beg);

        string tmp_string;
        string result_string;
        pair<string, string> my_pair;
        for (; input_file_stream.good();) {
            getline(input_file_stream, tmp_string);
            if (tmp_string.size() > 0) {
                my_pair = split(tmp_string);
                if (my_pair.first == key) {
                    result_string = std::move(my_pair.second);
                }
            }
        }
        db_file_array_[file_hash_index] = make_unique<fstream>(to_string(file_hash_index),
                                                               ios::in | ios::out | ios::app | ios::binary);
        if (result_string.size() > 0)
            return result_string;
        return "NULL";
    }

    //Can be optimized with first read, remove duplicate and then write whole
    inline void put(string &&key, string &&value) { //存储KV
        size_t file_hash_index = get_hash_file_index(key);
        auto &output_stream = *db_file_array_[file_hash_index];
        output_stream << key << ',' << value << '\n' << flush;
    }
};

#endif //INMEMORYKEYVALUESTOREWITHPERSISTENCE_KEY_VALUE_STORE_H