//
// Created by cheyulin on 8/16/16.
//

#ifndef KEYVALUESTORE_SIMPLE_KEY_VALUE_STORE_H_H
#define KEYVALUESTORE_SIMPLE_KEY_VALUE_STORE_H_H


#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <algorithm>

#define FILE_NAME "yche.db"

using namespace std;


class Answer {
private:
    unordered_map<string, string> yche_map;
    fstream yche_stream_;

public:
    Answer() {
        yche_map.reserve(60000);
        yche_stream_.open(FILE_NAME, ios::in | ios::out | ios::app | ios::binary);
        string key_string;
        string value_string
        for (; yche_stream_.good();) {
            getline(yche_stream_, key_string);
            if (yche_stream_.good()) {
                getline(yche_stream_,value_string);
                yche_map[key_string]=value_string;
            }
        }
        yche_stream_.clear();
    }

    string get(string key) {
        if (yche_map.find(key) != yche_map.end()) {
            return yche_map[key];
        } else {
            return "NULL";
        }
    }

    void put(string key, string value) { //存储KV
        yche_map[key] = value;
        yche_stream_ << key << "\n" << value << "\n";
        yche_stream_ << flush;
    }
};

#endif //KEYVALUESTORE_SIMPLE_KEY_VALUE_STORE_H_H
