//
// Created by cheyulin on 8/10/16.
//

#ifndef KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H
#define KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include "stdlib.h"
#include "unistd.h"

using namespace std;


#define FILE_NAME "tuple_transaction.db"
#define SEPERATOR ","
#define SEPERATOR_END_CHAR ';'
#define SEPERATOR_END_STRING ";"
#define SEPERATOR_CHAR ','

class Answer {
private:
    unordered_map<string, string> yche_map_;
    fstream output_file_stream_;
    size_t count{0};

    pair<string, string> split(const string &str) {
        pair<string, string> result;
        auto iter_begin = str.begin();
        auto iter_end = str.end();
        auto iter_middle = find(iter_begin, iter_end, ',');
        return std::move(make_pair(std::move(string(iter_begin, iter_middle)),
                                   std::move(string(iter_middle + 1, iter_end - 1))));
    }

public: //put和get方法要求public
    Answer() {
        fstream input_file_stream{FILE_NAME, ifstream::in};
        string tmp_string;
        if (input_file_stream.is_open()) {
            for (; !input_file_stream.eof();) {
                getline(input_file_stream, tmp_string);
                if (tmp_string.size() > 0 && tmp_string.substr(tmp_string.size() - 1) == SEPERATOR_END_STRING) {
                    auto my_pair = std::move(split(tmp_string));
                    yche_map_[my_pair.first] = my_pair.second;
                }
            }

        }
        input_file_stream.close();
        output_file_stream_.open(FILE_NAME, ofstream::app);
        output_file_stream_ << unitbuf;
    }

    virtual ~Answer() {
        if (output_file_stream_.is_open())
            output_file_stream_.close();
    }

    string get(string key) { //读取KV
        auto iter = yche_map_.find(key);
        if (iter != yche_map_.end()) {
            return iter->second;
        }
        else {
            return "NULL"; //文件不存在，说明该Key不存在，返回NULL
        }
    }

    void put(string key, string value) { //存储KV
        yche_map_[key] = value;
        count++;
        if (count < 100)
            output_file_stream_ << key << SEPERATOR << value << SEPERATOR_END_CHAR << endl;
        else if (count == 100)
            output_file_stream_.close();
    }
};

#endif //KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H