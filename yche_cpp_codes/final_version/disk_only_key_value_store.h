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

#define INDEX_FILE_NAME "index.meta"
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
    unordered_map<string, pair<int, int>> yche_map_;
    fstream key_index_stream_;
    fstream db_stream_;
    circular_buff *circular_buff_ptr_;

    int prefix_sum_index_{0};
    int length_{0};

    char *value_buffer;
    bool is_first_in_{true};

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
                if (is_first_in_) {
                    init_map_info(length_);
                    init_circular_buffer(length_);
                    is_first_in_ = false;
                }
                yche_map_[key_str] = make_pair(prefix_sum_index_, length_);
            }
        }
        prefix_sum_index_ = prefix_sum_index_ + length_;
        key_index_stream_.clear();
    }

    inline void init_map_info(int length) {
        if (length < 500) {
            yche_map_.reserve(60000);
        } else if (length < 5000) {
            yche_map_.reserve(1500000);
        } else {
            yche_map_.reserve(60000);
        }
    }

    inline void init_circular_buffer(int length) {
        if (length < 500) {
            circular_buff_ptr_ = new circular_buff(SMALL_BUFFER_SIZE, 160, prefix_sum_index_);
        } else if (length < 5000) {
            circular_buff_ptr_ = new circular_buff(MEDIUM_BUFFER_SIZE, 3000, prefix_sum_index_);
        } else {
            circular_buff_ptr_ = new circular_buff(BIG_BUFFER_SIZE, 30000, prefix_sum_index_);
        }
    }

public:
    Answer() {
        value_buffer = new char[1024 * 32];
        key_index_stream_.open(INDEX_FILE_NAME, ios::in | ios::out | ios::app | ios::binary);
        db_stream_.open(DB_NAME, ios::in | ios::out | ios::app | ios::binary);
        read_index_info();
    }

    virtual ~Answer() {
        delete circular_buff_ptr_;
        delete[] value_buffer;
    }

    inline string get(string key) {
        if (yche_map_.find(key) == yche_map_.end()) {
            return "NULL";
        }
        else {
            auto &index_pair = yche_map_[key];
            auto ptr = circular_buff_ptr_->peek_info(index_pair.first);
            if (ptr != nullptr) {
                cout << "Hit" << endl;
                return string(ptr, 0, index_pair.second);
            }
            else {
                db_stream_.seekg(index_pair.first, ios::beg);
                db_stream_.read(value_buffer, index_pair.second);
                return string(value_buffer, 0, index_pair.second);

            }
        }
    }

    inline void put(string key, string value) {
        length_ = value.size();
        if (is_first_in_) {
            init_map_info(length_);
            init_circular_buffer(length_);
            is_first_in_ = false;
        }
        key_index_stream_ << key << "\n";
        key_index_stream_ << prefix_sum_index_ << "\n";
        key_index_stream_ << length_ << "\n" << flush;

        db_stream_.seekp(0, ios::end);
        db_stream_ << value << flush;

        circular_buff_ptr_->push_back(value.c_str(), length_);
        yche_map_[key] = make_pair(prefix_sum_index_, length_);
        prefix_sum_index_ += length_;

    }
};

#endif //KEYVALUESTORE_DISK_ONLY_KEY_VALUE_STORE_H