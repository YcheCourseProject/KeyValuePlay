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

public:
    Answer() {
        yche_map_.reserve(60000);
        fstream input_file_stream{FILE_NAME, ios::in | ios::out | ios::app | ios::binary};
        string key_str;
        string value_str;
        for (; input_file_stream.good();) {
            getline(input_file_stream, key_str);
            if (input_file_stream.good()) {
                getline(input_file_stream, value_str);
                yche_map_[key_str] = value_str;
            }
        }
        db_file_stream_.open(FILE_NAME, ios::out | ios::app | ios::binary);
    }

    inline string get(string &&key) {
        auto result = yche_map_.find(key);
        if (result != yche_map_.end()) {
            return result->second;
        }
        else {
            return "NULL";
        }
    }

    inline void put(string &&key, string &&value) {
        db_file_stream_ << key << '\n' << value << '\n' << flush;
        yche_map_[key] = value;
    }
};

#endif //KEYVALUESTORE_ANOTHER_SIMPLE_KEY_VALUE_H
