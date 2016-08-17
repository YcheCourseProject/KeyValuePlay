//
// Created by cheyulin on 8/17/16.
//

#ifndef KEYVALUESTORE_DEFAULT_KEY_VALUE_STORE_H
#define KEYVALUESTORE_DEFAULT_KEY_VALUE_STORE_H

#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <sys/stat.h>
#include <sstream>
#include <map>

#define INDEX_FILE_NAME "key_file_name_index_file.txt"

using namespace std;

string get_file_name(int file_name_num) {
    return to_string(file_name_num);
}

class Answer {
private:
    unordered_map<string, string> key_file_name_map_;
    map<string, string> key_value_map_;
    fstream key_filename_stream_;
    int file_name_num_{0};
//    int cache_max_size_{1000};

public:
    Answer() {
        key_filename_stream_.open(INDEX_FILE_NAME, ios::in | ios::out | ios::app);
//        if (key_filename_stream_.good() == false) {
//            for (auto i = 0; i < 2000; i++) {
//                struct stat st = {0};
//                if (stat(to_string(i).c_str(), &st) == -1) {
//                    mkdir(to_string(i).c_str(), 0755);
//                }
//            }
//        }
        string key_str;
        string file_name_str;
        for (; key_filename_stream_.good();) {
            getline(key_filename_stream_, key_str);
            if (key_filename_stream_.good()) {
                getline(key_filename_stream_, file_name_str);
                key_file_name_map_[key_str] = file_name_str;
                file_name_num_++;
            }
        }
        key_filename_stream_.clear();
    }

    string get(string key) {
        if (key_file_name_map_.find(key) == key_file_name_map_.end()) {
            return "NULL";
        } else {
            if (key_value_map_.find(key) != key_value_map_.end()) {
                return key_value_map_[key];
            }
            ifstream is(key_file_name_map_[key]);
            string value;
            is >> value;
            is.close();

            return value;
        }
    }

    void put(string key, string value) {
        if (key_file_name_map_.find(key) == key_file_name_map_.end()) {
            key_filename_stream_ << key << "\n" << get_file_name(file_name_num_) << "\n" << flush;
            key_file_name_map_[key] = get_file_name(file_name_num_);
            file_name_num_++;
        }
        if (key_value_map_.find(key) != key_value_map_.end() || key_value_map_.size() < cache_max_size_)
            key_value_map_[key] = value;
        ofstream os(key_file_name_map_[key]);
        os << value;
        os.close();
    }
};

#endif //KEYVALUESTORE_DEFAULT_KEY_VALUE_STORE_H
