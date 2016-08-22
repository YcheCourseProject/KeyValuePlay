//
// Created by cheyulin on 8/20/16.
//

#ifndef KEYVALUESTORE_EFFICIENT_IO_KEY_VALUE_STORE_H
#define KEYVALUESTORE_EFFICIENT_IO_KEY_VALUE_STORE_H

#include <unordered_map>
#include <algorithm>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>

#define INDEX_FILE_NAME "index.meta"
#define DB_NAME "value.db"

#define MEDIUM_BUFFER_SIZE 200000000
#define BIG_BUFFER_SIZE 120000000
#define SMALL_BUFFER_SIZE 50000000

using namespace std;

class circular_buff {
private:
    int end_index_{0};
    int buffer_size_;
    bool is_full_{false};
    char *buffer_{nullptr};

public:
    circular_buff() = default;

    int file_offset_begin_;

    inline circular_buff(int buffer_size, int extra_space_size, int file_offset_begin = 0) {
        buffer_ = new char[buffer_size + extra_space_size];
        buffer_size_ = buffer_size;
        file_offset_begin_ = file_offset_begin;
    }

    inline circular_buff &operator=(circular_buff &&circular_buffer) {
        if (buffer_ != nullptr)
            delete[]buffer_;
        buffer_ = circular_buffer.buffer_;
        circular_buffer.buffer_ = nullptr;
        is_full_ = circular_buffer.is_full_;
        end_index_ = circular_buffer.end_index_;
        buffer_size_ = circular_buffer.buffer_size_;
    }

    inline ~circular_buff() {
        if (buffer_ != nullptr)
            delete[]buffer_;
    }

    inline void push_back(char *value, int size) {
        if (is_full_) {
            file_offset_begin_ += size;
        } else if (end_index_ + size > buffer_size_ - 1) {
            is_full_ = true;
        }
        memcpy(buffer_ + end_index_, value, size);
        end_index_ = (end_index_ + size) % buffer_size_;
    }

    inline char *peek_info(int offset) {
        if (offset >= file_offset_begin_)
            return buffer_ + ((offset - file_offset_begin_) % buffer_size_);
        else
            return nullptr;
    }
};

class Answer {
private:
    unordered_map<string, pair<int, int>> key_index_info_map_;
    fstream key_index_stream_;
    fstream db_stream_;
    circular_buff value_circular_buffer_;

    int prefix_sum_index_{0};
    int length_{0};
    char *value_buffer;
    bool is_initialized_{false};

    inline void init_map_info(int length) {
        if (length < 500) {
            key_index_info_map_.reserve(60000);
        } else if (length < 5000) {
            key_index_info_map_.reserve(60000);
        } else {
            key_index_info_map_.reserve(60000);
        }
    }

    inline void init_circular_buffer(int length) {
        if (length < 500) {
            value_circular_buffer_ = move(circular_buff(SMALL_BUFFER_SIZE, 160));
        } else if (length < 5000) {
            value_circular_buffer_ = move(circular_buff(MEDIUM_BUFFER_SIZE, 3000));
        } else {
            value_circular_buffer_ = move(circular_buff(BIG_BUFFER_SIZE, 30000));
        }
    }

    inline void read_index_info() {
        string key_str;
        string prefix_sum_index_str;
        string length_str;
        for (; key_index_stream_.good();) {
            getline(key_index_stream_, key_str);
            if (key_index_stream_.good()) {
                getline(key_index_stream_, prefix_sum_index_str);
                getline(key_index_stream_, length_str);
                prefix_sum_index_ = stoi(prefix_sum_index_str);
                length_ = stoi(length_str);
                if (is_initialized_ == false) {
                    is_initialized_ = true;
                    init_map_info(length_);
                }
                key_index_info_map_[key_str] = make_pair(prefix_sum_index_, length_);
            }
        }

        if (is_initialized_)
            init_circular_buffer(length_);
        prefix_sum_index_ = prefix_sum_index_ + length_;
        value_circular_buffer_.file_offset_begin_ = prefix_sum_index_;
        key_index_stream_.clear();
    }

public:
    Answer() {
        key_index_info_map_.reserve(20000);
        value_buffer = new char[1024 * 32];
        key_index_stream_.open(INDEX_FILE_NAME, ios::in | ios::out | ios::app | ios::binary);
        db_stream_.open(DB_NAME, ios::in | ios::out | ios::app | ios::binary);
        read_index_info();
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
            char *ptr = value_circular_buffer_.peek_info(index_pair.first);
//            if (ptr != nullptr)
//                return string(ptr, 0, index_pair.second);
//            else {
                db_stream_.seekg(index_pair.first, ios::beg);
                db_stream_.read(value_buffer, index_pair.second);
                return string(value_buffer, 0, index_pair.second);
//            }
        }
    }

    inline void put(string key, string value) {
        length_ = value.size();
        if (is_initialized_ == false) {
            is_initialized_ = true;
            init_map_info(length_);
            init_circular_buffer(length_);
        }
        key_index_stream_ << key << "\n";
        key_index_stream_ << prefix_sum_index_ << "\n";
        key_index_stream_ << length_ << "\n" << flush;

        db_stream_.seekp(0, ios::end);
        db_stream_ << value << flush;
        value_circular_buffer_.push_back((char *) value.c_str(), length_);
        key_index_info_map_[key] = make_pair(prefix_sum_index_, length_);
        prefix_sum_index_ += length_;

    }
};

#endif //KEYVALUESTORE_EFFICIENT_IO_KEY_VALUE_STORE_H