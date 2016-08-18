//
// Created by cheyulin on 8/17/16.
//

#ifndef KEYVALUESTORE_DEFAULT_KEY_VALUE_STORE_H
#define KEYVALUESTORE_DEFAULT_KEY_VALUE_STORE_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <map>
#include <iomanip>
#include <algorithm>

#define INDEX_FILE_NAME "index.meta"
#define SMALL_DB_NAME "small.db"
#define MEDIUM_DB_NAME "medium.db"
#define LARGE_DB_NAME "large.db"

//#define SMALL_KEY_ALIGNMENT 70
#define SMALL_KEY_ALIGNMENT 96
//#define MEDIUM_KEY_ALIGNMENT 300
#define MEDIUM_KEY_ALIGNMENT 584
//#define LARGE_KEY_ALIGNMENT 3000
#define LARGE_KEY_ALIGNMENT 3280

#define SMALL_VALUE_ALIGNMENT 160
#define MEDIUM_VALUE_ALIGNMENT 3000
#define LARGE_VALUE_ALIGNMENT 30000
#define BUFFER_SIZE 1023

using namespace std;

class Answer {
private:
    unordered_map<string, int> key_index_map_;
    unordered_map<string, string> key_value_map_;
    fstream key_index_stream_;
    fstream db_stream_;
    int db_file_index_{0};

    int key_alignment_{0};
    int value_alignment_{0};
    int cache_max_size_{1000};
    int block_size_{1};
    bool is_first_in_{false};
    char *buffer_chars_;

    inline void trim_right_blank(string &to_trim_string) {
        auto iter_back = find_if_not(to_trim_string.rbegin(), to_trim_string.rend(),
                                     [](int c) { return isspace(c); }).base();
        to_trim_string = string(to_trim_string.begin(), iter_back);
    }

    inline void set_cache_max_size() {
        if (value_alignment_ == SMALL_VALUE_ALIGNMENT) {
            cache_max_size_ = 100000;
        } else if (value_alignment_ == MEDIUM_VALUE_ALIGNMENT) {
            cache_max_size_ = 60000;
        } else {
            cache_max_size_ = 6000;
        }
    }

    inline void set_block_buffer_size() {
        block_size_ = BUFFER_SIZE / (key_alignment_ + value_alignment_);
    }

    inline void read_index_info() {
        string key_str;
        string index_str;
        for (; key_index_stream_.good();) {
            getline(key_index_stream_, key_str);
            if (key_index_stream_.good()) {
                getline(key_index_stream_, index_str);
                key_index_map_[key_str] = std::stoi(index_str);
                db_file_index_++;
            }
        }
        key_index_stream_.clear();
    }

    inline void read_db_info() {
        key_index_stream_.open(INDEX_FILE_NAME, ios::in | ios::out | ios::app | ios::binary);
        db_stream_.open(SMALL_DB_NAME, ios::in | ios::out | ios::binary);
        if (db_stream_.good() == true) {
            key_alignment_ = SMALL_KEY_ALIGNMENT;
            value_alignment_ = SMALL_VALUE_ALIGNMENT;
        } else {
            db_stream_.open(MEDIUM_DB_NAME, ios::in | ios::out | ios::binary);
            if (db_stream_.good() == true) {
                key_alignment_ = MEDIUM_KEY_ALIGNMENT;
                value_alignment_ = MEDIUM_VALUE_ALIGNMENT;
            } else {
                db_stream_.open(LARGE_DB_NAME, ios::in | ios::out | ios::binary);
                if (db_stream_.good() == true) {
                    key_alignment_ = LARGE_KEY_ALIGNMENT;
                    value_alignment_ = LARGE_VALUE_ALIGNMENT;
                }
                else {
                    is_first_in_ = true;
                }
            }
        }
        if (is_first_in_ == false) {
//            if (value_alignment_ == SMALL_VALUE_ALIGNMENT) {
//                read_some_buffer_info();
//            }
            set_cache_max_size();
            set_block_buffer_size();
        }
    }

    inline void read_some_buffer_info() {
        set_cache_max_size();
        char *key_string = new char[key_alignment_];
        char *value_string = new char[value_alignment_];
        string key;
        string value;
        for (auto i = 0; key_value_map_.size() < cache_max_size_; i++) {
            db_stream_.seekg(i * (value_alignment_ + key_alignment_), ios::beg);
            db_stream_.read(key_string, key_alignment_);
            if (db_stream_.good() == false) {
                db_stream_.clear();
                break;
            }
            db_stream_.seekg(i * (value_alignment_ + key_alignment_) + key_alignment_, ios::beg);
            db_stream_.read(value_string, value_alignment_);
            key = string(key_string);
            trim_right_blank(key);
            value = string(value_string);
            trim_right_blank(value);
            key_value_map_[key] = value;
        }
        db_stream_.clear();
        delete[] key_string;
        delete[] value_string;
    }

    inline void init_db_file(const string &value) {
        string db_file_name;
        if (value.size() <= SMALL_VALUE_ALIGNMENT) {
            db_file_name = SMALL_DB_NAME;
            key_alignment_ = SMALL_KEY_ALIGNMENT;
            value_alignment_ = SMALL_VALUE_ALIGNMENT;
        } else if (value.size() <= MEDIUM_VALUE_ALIGNMENT) {
            db_file_name = MEDIUM_DB_NAME;
            key_alignment_ = MEDIUM_KEY_ALIGNMENT;
            value_alignment_ = MEDIUM_VALUE_ALIGNMENT;
        } else {
            db_file_name = LARGE_DB_NAME;
            key_alignment_ = LARGE_KEY_ALIGNMENT;
            value_alignment_ = LARGE_VALUE_ALIGNMENT;
        }
        db_stream_.open(db_file_name, ios::out | ios::trunc);
        db_stream_.close();
        db_stream_.open(db_file_name, ios::in | ios::out | ios::binary);
        set_cache_max_size();
        set_block_buffer_size();
    }

public:
    Answer() {
        buffer_chars_ = new char[BUFFER_SIZE];
        read_db_info();
        read_index_info();
    }

    virtual ~Answer() {
        cout << "~Answer" << endl;
        delete[]buffer_chars_;
    }

    inline string get(string key) {
        if (key_index_map_.find(key) == key_index_map_.end()) {
            return "NULL";
        }
        else {
            if (key_value_map_.find(key) != key_value_map_.end()) {
                return key_value_map_[key];
            }
            if (key_index_map_.size() < cache_max_size_) {
                auto cur_index = key_index_map_[key];
                auto final_index = key_index_map_.size() - 1;
                auto stop_index = cur_index + block_size_ > final_index ? final_index : cur_index + block_size_;
                auto available_block_size = stop_index - cur_index + 1;
                db_stream_.seekg(cur_index * (key_alignment_ + value_alignment_), ios::beg);
                db_stream_.read(buffer_chars_, (key_alignment_ + value_alignment_) * available_block_size);
                string key_string;
                string value_string;
                string result_string(buffer_chars_, key_alignment_, value_alignment_);
                trim_right_blank(result_string);
                for (auto i = 0; i < available_block_size && key_value_map_.size() < cache_max_size_; i++) {
                    key_string = string(buffer_chars_, i * (key_alignment_ + value_alignment_),
                                        key_alignment_);
                    trim_right_blank(key_string);
                    value_string =
                            string(buffer_chars_, i * (key_alignment_ + value_alignment_) + key_alignment_,
                                   value_alignment_);
                    trim_right_blank(value_string);
                    key_value_map_[key_string] = value_string;
//                    cout << "Key:" << key_string << "," << "Val:" << value_string << ";" << endl;
                }
                return result_string;
            } else {
                db_stream_.seekg(key_index_map_[key] * (key_alignment_ + value_alignment_) + key_alignment_, ios::beg);
                db_stream_.read(buffer_chars_, value_alignment_);
                string result_string(buffer_chars_, 0, value_alignment_);
                trim_right_blank(result_string);
                return result_string;
            }
        }
    }

    inline void put(string key, string value) {
        if (is_first_in_) {
            init_db_file(value);
            is_first_in_ = false;
        }
        if (key_index_map_.find(key) == key_index_map_.end()) {
            key_index_stream_ << key << "\n" << to_string(db_file_index_) << "\n" << flush;
            key_index_map_[key] = db_file_index_;
            db_file_index_++;
            db_stream_.seekp(0, ios::end);
        } else {
            db_stream_.seekp(key_index_map_[key] * (key_alignment_ + value_alignment_), ios::beg);
        }
        db_stream_ << left << setw(key_alignment_) << key << left << setw(value_alignment_) << value << flush;

        if (key_value_map_.size() < cache_max_size_ || key_value_map_.find(key) != key_value_map_.end())
            key_value_map_[key] = value;
    }
};

#endif //KEYVALUESTORE_DEFAULT_KEY_VALUE_STORE_H
