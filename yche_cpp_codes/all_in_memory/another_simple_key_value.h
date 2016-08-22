//
// Created by cheyulin on 8/21/16.
//

#ifndef KEYVALUESTORE_ANOTHER_SIMPLE_KEY_VALUE_H
#define KEYVALUESTORE_ANOTHER_SIMPLE_KEY_VALUE_H

#include <string>
#include <fstream>
#include <algorithm>
#include <unordered_map>

#define FILE_NAME "transaction.db"

#include <cstring>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

class Answer {
private:
    unordered_map<string, string> yche_map_;
    int file_descriptor_;
    char *mmap_;
    int index_{0};

public:
    inline Answer() {
        yche_map_.reserve(60000);
        fstream input_file_stream{FILE_NAME, ios::in | ios::binary};
        char *buffer = new char[6000000];
        input_file_stream.rdbuf()->pubsetbuf(buffer, 6000000);
        string key_str;
        string value_str;
        for (; input_file_stream.good();) {
            getline(input_file_stream, key_str);
            if (input_file_stream.good()) {
                getline(input_file_stream, value_str);
                yche_map_[key_str] = value_str;
                index_ += key_str.size() + value_str.size() + 2;
            }
        }
        delete[]buffer;
        input_file_stream.close();
        file_descriptor_ = open(FILE_NAME, O_RDWR | O_CREAT, 0600);
        ftruncate(file_descriptor_, 6000000);
        mmap_ = (char *) mmap(0, 6000000, PROT_WRITE, MAP_SHARED, file_descriptor_, 0);
    }

    inline string get(string key) {
        auto result = yche_map_.find(key);
        if (result != yche_map_.end()) {
            return result->second;
        }
        else {
            return "NULL";
        }
    }

    inline void put(string key, string value) {
        memcpy(mmap_ + index_, key.c_str(), key.size());
        mmap_[index_ + key.size()] = '\n';
        memcpy(mmap_ + index_ + key.size() + 1, value.c_str(), value.size());
        mmap_[index_ + key.size() + value.size() + 1] = '\n';
        index_ += key.size() + value.size() + 2;
        yche_map_[key] = value;
    }
};

#endif //KEYVALUESTORE_ANOTHER_SIMPLE_KEY_VALUE_H
