//
// Created by cheyulin on 8/17/16.
//

#ifndef KEYVALUESTORE_DEFAULT_KEY_VALUE_STORE_H
#define KEYVALUESTORE_DEFAULT_KEY_VALUE_STORE_H

#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <unordered_map>

using namespace std;

template<typename _Tp, typename... _Args>
inline unique_ptr<_Tp> make_unique(_Args &&... __args) {
    return unique_ptr<_Tp>(new _Tp(std::forward<_Args>(__args)...));
}

std::hash<string> str_hash_func_basic;

class Answer {
    array<fstream, 1000> my_file_streams_;
    array<bool, 1000> mark_flags_;
    unordered_map<string, int> key_set_;

public:
    Answer() {

    }

    string get(string key) {
        ifstream is(key);
        if (is.good()) {
            string value;
            is >> value;
            is.close();
            return value;
        } else {
            return "NULL";
        }
    }

    void put(string key, string value) { //存储KV
        ofstream os(key);
        os << value << flush;
        os.close();
    }
};

#endif //KEYVALUESTORE_DEFAULT_KEY_VALUE_STORE_H
