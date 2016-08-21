//
// Created by cheyulin on 8/21/16.
//

#ifndef KEYVALUESTORE_ANOTHER_SIMPLE_KEY_VALUE_H
#define KEYVALUESTORE_ANOTHER_SIMPLE_KEY_VALUE_H

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <unordered_map>

#define FILE_NAME "tuple_transaction.db"

using namespace std;

class Answer {
private:
    unordered_map<string, string> yche_map_;
    fstream db_file_stream_;
    char *db_value_mmap_;

    inline pair<string, string> split(const string &str) {
        auto iter_begin = str.begin();
        auto iter_end = str.end();
        auto iter_middle = find(iter_begin, iter_end, ',');
        return make_pair(string(iter_begin, iter_middle),
                         string(iter_middle + 1, iter_end));
    }

public:
    Answer() {
        yche_map_.reserve(300000);
        fstream input_file_stream{FILE_NAME, ios::in | ios::out | ios::app | ios::binary};
        string tmp_string;
        for (; input_file_stream.good();) {
            getline(input_file_stream, tmp_string);
            if (input_file_stream.good()) {
                auto my_pair = split(tmp_string);
                yche_map_[my_pair.first] = my_pair.second;
            }
        }
        //invalidate due to eof flag
        db_file_stream_.open(FILE_NAME, ios::out | ios::app | ios::binary);
    }

    inline string get(string &&key) { //读取KV
        auto result = yche_map_.find(key);
        if (result != yche_map_.end()) {
            return result->second;
        }
        else {
            return "NULL"; //文件不存在，说明该Key不存在，返回NULL
        }
    }

    inline void put(string &&key, string &&value) { //存储KV
        db_file_stream_ << key << ',' << value << '\n' << flush;
        yche_map_[key] = value;
    }
};


#endif //KEYVALUESTORE_ANOTHER_SIMPLE_KEY_VALUE_H
