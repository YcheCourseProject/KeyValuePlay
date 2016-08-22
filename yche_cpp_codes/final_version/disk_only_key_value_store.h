//
// Created by cheyulin on 8/22/16.
//

#ifndef KEYVALUESTORE_DISK_ONLY_KEY_VALUE_STORE_H
#define KEYVALUESTORE_DISK_ONLY_KEY_VALUE_STORE_H

#include <unordered_map>
#include <algorithm>
#include <string>
#include <fstream>

#include <cstring>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>

#define INDEX_FILE_NAME "index.meta"
#define DB_NAME "value.db"

using namespace std;

class Answer {
private:
    unordered_map<string, pair<int, int>> key_index_info_map_;

    fstream db_stream_;
    int file_descriptor_{-1};
    char *mmap_{nullptr};
    int key_index_{0};

    int prefix_sum_index_{0};
    int length_{0};
    char *value_buffer;
    bool is_initialized_{false};

    inline void read_index_info() {
        cout << "Go" << endl;
        fstream key_index_stream;
        key_index_stream.open(INDEX_FILE_NAME, ios::in | ios::binary);
        string key_str;
        string prefix_sum_index_str;
        string length_str;
        for (; key_index_stream.good();) {
            getline(key_index_stream, key_str);
            if (key_index_stream.good()) {
                getline(key_index_stream, prefix_sum_index_str);
                getline(key_index_stream, length_str);
                prefix_sum_index_ = stoi(prefix_sum_index_str);
                length_ = stoi(length_str);
                if (is_initialized_ == false) {
                    is_initialized_ = true;
                    init_map_info(length_);
                    struct stat status_block;
                    fstat(file_descriptor_, &status_block);
                    file_descriptor_ = open(INDEX_FILE_NAME, O_RDWR | O_CREAT, 0600);
                    mmap_ = (char *) mmap(0, status_block.st_size, PROT_WRITE, MAP_SHARED, file_descriptor_, 0);
                }
                key_index_info_map_[key_str] = make_pair(prefix_sum_index_, length_);
            }
        }
        prefix_sum_index_ = prefix_sum_index_ + length_;
        key_index_ = prefix_sum_index_;
    }

    inline void init_map_info(int length) {
        if (length < 500) {
            key_index_info_map_.reserve(60000);
        } else if (length < 5000) {
            key_index_info_map_.reserve(60000);
        } else {
            key_index_info_map_.reserve(60000);
        }
    }

    inline void truncate_key_index_file(int length) {
        auto size = 0;
        if (length < 500) {
            ftruncate(file_descriptor_, 2000000);
            size = 2000000;
        } else if (length < 5000) {
            ftruncate(file_descriptor_, 110000000);
            size = 110000000;
        } else {
            ftruncate(file_descriptor_, 185000000);
            size = 185000000;
        }
        mmap_ = (char *) mmap(0, size, PROT_WRITE, MAP_SHARED, file_descriptor_, 0);
    }

public:
    Answer() {
        value_buffer = new char[1024 * 32];
        read_index_info();
        db_stream_.open(DB_NAME, ios::in | ios::out | ios::app | ios::binary);

    }

    virtual ~Answer() {
        delete[] value_buffer;
    }

    inline string get(string key) {
        if (key_index_info_map_.find(key) == key_index_info_map_.end()) {
            return "NULL";
        }
        else {
            auto &index_pair = key_index_info_map_[key];
            db_stream_.seekg(index_pair.first, ios::beg);
            db_stream_.read(value_buffer, index_pair.second);
            return string(value_buffer, 0, index_pair.second);
        }
    }

    inline void put(string key, string value) {
        length_ = value.size();
        if (is_initialized_ == false) {
            init_map_info(length_);
        }
        if (key_index_ == 0) {
            truncate_key_index_file(length_);
        }
        string prefix_sum_str = to_string(prefix_sum_index_);
        string length_str = to_string(length_);
        memcpy(mmap_ + key_index_, key.c_str(), key.size());
        mmap_[key_index_ + key.size()] = '\n';

        memcpy(mmap_ + key_index_ + key.size() + 1, prefix_sum_str.c_str(), prefix_sum_str.size());
        mmap_[key_index_ + key.size() + 1 + prefix_sum_str.size()] = '\n';
        memcpy(mmap_ + key_index_ + key.size() + 2 + prefix_sum_str.size(), length_str.c_str(), length_str.size());
        mmap_[key_index_ + key.size() + 2 + prefix_sum_str.size() + length_str.size()] = '\n';
        key_index_ += key.size() + to_string(prefix_sum_index_).size() + to_string(length_).size() + 3;

        db_stream_.seekp(0, ios::end);
        db_stream_ << value << flush;

        key_index_info_map_[key] = make_pair(prefix_sum_index_, length_);
        prefix_sum_index_ += length_;

    }
};

#endif //KEYVALUESTORE_DISK_ONLY_KEY_VALUE_STORE_H
