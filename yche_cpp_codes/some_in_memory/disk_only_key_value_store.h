//
// Created by cheyulin on 8/22/16.
//

#ifndef KEYVALUESTORE_DISK_ONLY_KEY_VALUE_STORE_H
#define KEYVALUESTORE_DISK_ONLY_KEY_VALUE_STORE_H

#include <unordered_map>
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>

#define INDEX_NAME "index.meta"
#define DB_NAME "value.db"

#define MEDIUM_BUFFER_SIZE 150000000
#define BIG_BUFFER_SIZE 120000000
#define SMALL_BUFFER_SIZE 330

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

    inline circular_buff &operator=(circular_buff &&right_circular_buffer) {
        if (buffer_ != nullptr)
            delete[]buffer_;
        buffer_ = right_circular_buffer.buffer_;
        right_circular_buffer.buffer_ = nullptr;
        is_full_ = right_circular_buffer.is_full_;
        end_index_ = right_circular_buffer.end_index_;
        buffer_size_ = right_circular_buffer.buffer_size_;
    }

    inline ~circular_buff() {
        cout << "~circular" << endl;
        if (buffer_ != nullptr)
            delete[]buffer_;
    }

    inline void push_back(const char *value, int size) {
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
    unordered_map<string, pair<int, int>> map_;
    fstream index_stream_;
    fstream val_stream_;
    circular_buff *circular_buff_ptr_;

    int val_index_{0};
    int val_len_{0};

    char *value_buffer;
    bool is_first_in_{true};

    inline void read_index_info() {
        string key_str;
        string prefix_sum_index_str;
        string length_str;
        for (; index_stream_.good();) {
            getline(index_stream_, key_str);
            if (index_stream_.good()) {
                getline(index_stream_, prefix_sum_index_str);
                getline(index_stream_, length_str);
                val_index_ = stoi(prefix_sum_index_str);
                val_len_ = stoi(length_str);
                if (is_first_in_) {
                    init_map_info(val_len_);
                    init_circular_buffer(val_len_);
                    is_first_in_ = false;
                }
                map_[key_str] = make_pair(val_index_, val_len_);
            }
        }
        val_index_ = val_index_ + val_len_;
        index_stream_.clear();
    }

    inline void init_map_info(int length) {
        if (length < 500) {
            map_.reserve(60000);
        } else if (length < 5000) {
            map_.reserve(1500000);
        } else {
            map_.reserve(60000);
        }
    }

    inline void init_circular_buffer(int length) {
        if (length < 500) {
            circular_buff_ptr_ = new circular_buff(SMALL_BUFFER_SIZE, 160, val_index_);
        } else if (length < 5000) {
            circular_buff_ptr_ = new circular_buff(MEDIUM_BUFFER_SIZE, 3000, val_index_);
        } else {
            circular_buff_ptr_ = new circular_buff(BIG_BUFFER_SIZE, 30000, val_index_);
        }
    }

public:
    Answer() {
        value_buffer = new char[1024 * 32];
        index_stream_.open(INDEX_NAME, ios::in | ios::out | ios::app | ios::binary);
        val_stream_.open(DB_NAME, ios::in | ios::out | ios::app | ios::binary);
        read_index_info();
    }

    virtual ~Answer() {
        delete circular_buff_ptr_;
        delete[] value_buffer;
    }

    inline string get(string key) {
        if (map_.find(key) == map_.end()) {
            return "NULL";
        }
        else {
            auto &index_pair = map_[key];
            auto ptr = circular_buff_ptr_->peek_info(index_pair.first);
            if (ptr != nullptr) {
                cout << "Hit" << endl;
                return string(ptr, 0, index_pair.second);
            }
            else {
                val_stream_.seekg(index_pair.first, ios::beg);
                val_stream_.read(value_buffer, index_pair.second);
                return string(value_buffer, 0, index_pair.second);

            }
        }
    }

    inline void put(string key, string value) {
        val_len_ = value.size();
        if (is_first_in_) {
            init_map_info(val_len_);
            init_circular_buffer(val_len_);
            is_first_in_ = false;
        }
        index_stream_ << key << "\n";
        index_stream_ << val_index_ << "\n";
        index_stream_ << val_len_ << "\n" << flush;

        val_stream_.seekp(0, ios::end);
        val_stream_ << value << flush;

        circular_buff_ptr_->push_back(value.c_str(), val_len_);
        map_[key] = make_pair(val_index_, val_len_);
        val_index_ += val_len_;

    }
};

#endif //KEYVALUESTORE_DISK_ONLY_KEY_VALUE_STORE_H