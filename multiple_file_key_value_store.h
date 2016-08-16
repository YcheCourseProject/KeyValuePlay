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

using namespace std;

#define SEPERATOR_STRING ","
#define SEPERATOR_END_STRING ";"
#define HASH_FUNC(x) str_hash_func_basic(x)
#define DB_FILE_NUM 10000

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
public:
    Answer() {
    }

    inline string get(string &&key) { //读取KV
        size_t file_hash_index = get_hash_file_index(key);
        fstream input_file_stream(to_string(file_hash_index), ios::in | ios::binary);

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
        if (result_string.size() > 0)
            return result_string;
        return "NULL";
    }

    //Can be optimized with first read, remove duplicate and then write whole
    inline void put(string &&key, string &&value) { //存储KV
        size_t file_hash_index = get_hash_file_index(key);
        fstream output_stream(to_string(file_hash_index), ios::out | ios::app | ios::binary);
        output_stream << key << ',' << value << '\n' << flush;
    }
};

#endif //INMEMORYKEYVALUESTOREWITHPERSISTENCE_KEY_VALUE_STORE_H