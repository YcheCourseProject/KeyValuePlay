//
// Created by cheyulin on 8/17/16.
//

#ifndef KEYVALUESTORE_DEFAULT_KEY_VALUE_STORE_H
#define KEYVALUESTORE_DEFAULT_KEY_VALUE_STORE_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

#define INDEX_NAME "index.meta"
#define SMALL_DB_NAME "small.db"
#define MEDIUM_DB_NAME "medium.db"
#define LARGE_DB_NAME "large.db"

#define SMALL_KEY_ALIGNMENT 70
#define MEDIUM_KEY_ALIGNMENT 300
#define LARGE_KEY_ALIGNMENT 3000

#define SMALL_VALUE_ALIGNMENT 160
#define MEDIUM_VALUE_ALIGNMENT 3000
#define LARGE_VALUE_ALIGNMENT 30000
#define BUFFER_SIZE 1024*1024

#define HASH_FUNC(x) hash_func(x)
#define DEFAULT_HASH_SLOT_SIZE 10000

using namespace std;

std::hash<string> hash_func;

template<typename T>
class yche_map {
private:
    vector<pair<string, T>> hash_table_;
    size_t current_size_{0};
    size_t max_slot_size_{DEFAULT_HASH_SLOT_SIZE};

    inline void rebuild() {
        vector<pair<string, T>> my_hash_table_building;
        max_slot_size_ *= 2;
        my_hash_table_building.resize(max_slot_size_);
        for (auto previous_index = 0; previous_index < max_slot_size_ / 2; ++previous_index) {
            if (hash_table_[previous_index].first.size() > 0) {
                auto new_index = HASH_FUNC(hash_table_[previous_index].first) % max_slot_size_;
                for (; my_hash_table_building[new_index].first.size() != 0;
                       new_index = (++new_index) % max_slot_size_) {
                }
                my_hash_table_building[new_index] = std::move(hash_table_[previous_index]);
            }
        }
        hash_table_ = std::move(my_hash_table_building);
    }

public:
    yche_map() {
        reserve(DEFAULT_HASH_SLOT_SIZE);
    }

    inline void reserve(int size) {
        hash_table_.resize(size);
    }

    inline size_t size() {
        return current_size_;
    }

    inline T *find(const string &key) {
        auto index = HASH_FUNC(key) % max_slot_size_;
        //linear probing
        for (; hash_table_[index].first.size() != 0; index = (++index) % max_slot_size_) {
            if (hash_table_[index].first == key) {
                return &hash_table_[index].second;
            }
        }
        return nullptr;
    }

    inline void insert_or_replace(const string &key, const T &value) {
        auto index = HASH_FUNC(key) % max_slot_size_;
        for (; hash_table_[index].first.size() != 0; index = (++index) % max_slot_size_) {
            if (hash_table_[index].first == key) {
                hash_table_[index].second = value;
                return;
            }
        }
        ++current_size_;
        hash_table_[index].first = key;
        hash_table_[index].second = value;
        if (current_size_ / max_slot_size_ > 0.8) {
            rebuild();
        }
    }
};

class Answer {
private:
    yche_map<int> map_;
    yche_map<string> key_value_map_;
    fstream index_stream_;
    fstream db_stream_;
    int db_file_index_{0};

    int key_alignment_{0};
    int value_alignment_{0};
    int cache_max_size_{1000};
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
            cache_max_size_ = 150000;
        } else {
            cache_max_size_ = 9500;
        }
        key_value_map_.reserve(cache_max_size_ * 1.3);
        if (value_alignment_ == MEDIUM_VALUE_ALIGNMENT)
            map_.reserve(500000);
        else
            map_.reserve(100000);
    }

    inline void read_index_info() {
        string key_str;
        string index_str;
        for (; index_stream_.good();) {
            getline(index_stream_, key_str);
            if (index_stream_.good()) {
                getline(index_stream_, index_str);
                map_.insert_or_replace(key_str, stoi(index_str));
                db_file_index_++;
            }
        }
        index_stream_.clear();
    }

    inline void read_db_info() {
        index_stream_.open(INDEX_NAME, ios::in | ios::out | ios::app | ios::binary);
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
        }
    }

    inline void read_some_buffer_info() {
        set_cache_max_size();
        char *key_string = new char[key_alignment_];
        char *value_string = new char[value_alignment_];
        string key;
        string value;
        for (auto i = 0; key_value_map_.size() < cache_max_size_; ++i) {
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
            key_value_map_.insert_or_replace(key, value);
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
    }

public:
    Answer() {
        buffer_chars_ = new char[BUFFER_SIZE];
        read_db_info();
        read_index_info();
    }

    virtual ~Answer() {
        delete[]buffer_chars_;
    }

    inline string get(string &&key) {
        if (map_.find(key) == nullptr) {
            return "NULL";
        }
        else {
            auto value_ptr = key_value_map_.find(key);
            if (value_ptr != nullptr) {
                return *value_ptr;
            }
            db_stream_.seekg(*map_.find(key) * (key_alignment_ + value_alignment_) + key_alignment_,
                             ios::beg);
            db_stream_.read(buffer_chars_, value_alignment_);
            string result_string(buffer_chars_, 0, value_alignment_);
            trim_right_blank(result_string);
            return result_string;
        }
    }

    inline void put(string &&key, string &&value) {
        if (is_first_in_) {
            init_db_file(value);
            is_first_in_ = false;
        }
        if (map_.find(key) == nullptr) {
            index_stream_ << key << "\n" << to_string(db_file_index_) << "\n" << flush;
            map_.insert_or_replace(key, db_file_index_);
            db_file_index_++;
            db_stream_.seekp(0, ios::end);
        } else {
            db_stream_.seekp(*map_.find(key) * (key_alignment_ + value_alignment_), ios::beg);
        }
        db_stream_ << left << setw(key_alignment_) << key << left << setw(value_alignment_) << value << flush;
        if (key_value_map_.size() < cache_max_size_ || key_value_map_.find(key) != nullptr)
            key_value_map_.insert_or_replace(key, value);
    }
};

#endif //KEYVALUESTORE_DEFAULT_KEY_VALUE_STORE_H
