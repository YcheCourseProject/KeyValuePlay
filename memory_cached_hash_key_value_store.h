//
// Created by cheyulin on 8/10/16.
//

#ifndef KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H
#define KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>

using namespace std;

#define HASH_FUNC(x) str_hash_func_basic(x)
constexpr char *FILE_NAME = "tuple_transaction.db";
constexpr char *SEPERATOR_STRING = ",";
constexpr int DEFAULT_HASH_TABLE_SLOT_SIZE = 80000;

constexpr int EXTRA_ALIGNMENT_SIZE = 1;
constexpr char *extra_split_string = "\n";
constexpr char *SMALL_FILE_NAME = "small.db";
constexpr char *MEDIUM_FILE_NAME = "middle.db";
constexpr char *LARGE_FILE_NAME = "big.db";

enum class KeyAlignment {
    small = 70,
    medium = 300,
    large = 3000
};

enum class ValueAlignment {
    small = 160,
    medium = 3000,
    large = 30000
};

enum class HashFileSlotSize {
    small = 100000,
    medium = 1000000,
    large = 100000

};

enum class HashInMemoryMaxSize {
    small = 100000,
    medium = 90000,
    large = 9000
};

enum class DataSetType {
    small,
    medium,
    large
};

struct DataSetAlignmentInfo {
    int key_alignment_size_;
    int value_alignment_size_;
    int whole_alignment_size_;

    int hash_in_memory_tuple_size_;
    int hash_file_slot_size_;

    DataSetAlignmentInfo(DataSetType dataset_type) {
        if (dataset_type == DataSetType::small) {
            key_alignment_size_ = static_cast<int>(KeyAlignment::small);
            value_alignment_size_ = static_cast<int>(ValueAlignment::small);
            hash_file_slot_size_ = static_cast<int>(HashFileSlotSize::small);
            hash_in_memory_tuple_size_ = static_cast<int>(HashInMemoryMaxSize::small);
        } else if (dataset_type == DataSetType::medium) {
            key_alignment_size_ = static_cast<int>(KeyAlignment::medium);
            value_alignment_size_ = static_cast<int>(ValueAlignment::medium);
            hash_file_slot_size_ = static_cast<int>(HashFileSlotSize::medium);
            hash_in_memory_tuple_size_ = static_cast<int>(HashInMemoryMaxSize::medium);
        } else {
            key_alignment_size_ = static_cast<int>(KeyAlignment::large);
            value_alignment_size_ = static_cast<int>(ValueAlignment::large);
            hash_file_slot_size_ = static_cast<int>(HashFileSlotSize::large);
            hash_in_memory_tuple_size_ = static_cast<int>(HashInMemoryMaxSize::large);
        }
        whole_alignment_size_ = key_alignment_size_ + value_alignment_size_ + EXTRA_ALIGNMENT_SIZE;
    }
};

inline void trim_right_blank(string &to_trim_string) {
    auto iter_back = std::find_if_not(to_trim_string.rbegin(), to_trim_string.rend(),
                                      [](int c) { return std::isspace(c); }).base();
    to_trim_string = std::string(to_trim_string.begin(), iter_back);
}

std::hash<string> str_hash_func_basic;

template<size_t slot_num = DEFAULT_HASH_TABLE_SLOT_SIZE>
class yche_string_string_map {
private:
    vector<pair<string, string>> my_hash_table_;
    size_t current_size_{0};
    size_t slot_max_size_{slot_num};

    //structure for persistence
    bool is_current_in_memory_table_fulll_{false};


    inline void rebuild() {
        vector<pair<string, string>> my_hash_table_building;
        slot_max_size_ *= 2;
        my_hash_table_building.resize(slot_max_size_);
        for (auto previous_index = 0; previous_index < slot_max_size_ / 2; ++previous_index) {
            if (my_hash_table_[previous_index].first.size() > 0) {
                auto new_index = HASH_FUNC(my_hash_table_[previous_index].first) % slot_max_size_;
                for (; my_hash_table_building[new_index].first.size() != 0;
                       new_index = (++new_index) % slot_max_size_) {
                }
                my_hash_table_building[new_index] = std::move(my_hash_table_[previous_index]);
            }

        }
        my_hash_table_ = std::move(my_hash_table_building);
    }

    inline void write_key_value_to_file(const string &key, const string &value) {

    }

public:
    yche_string_string_map() : my_hash_table_(slot_num) {}

    DataSetAlignmentInfo *data_set_alignment_info_ptr_{nullptr};

    fstream *db_file_stream_ptr_{nullptr};

    inline size_t size() {
        return current_size_;
    }

    inline string *find(const string &key) {
        auto hash_result = HASH_FUNC(key);
        auto index = hash_result % slot_max_size_;
        //linear probing
        for (; my_hash_table_[index].first.size() != 0; index = (++index) % slot_max_size_) {
            if (my_hash_table_[index].first == key) {
                return &my_hash_table_[index].second;
            }
        }

        //if in-memory hash_table is full search in file, read file
        index = hash_result % data_set_alignment_info_ptr_->hash_file_slot_size_;
        //linear probing

        return nullptr;
    }

    inline void insert_or_replace(const string &key, const string &value) {
        auto index = HASH_FUNC(key) % slot_max_size_;
        for (; my_hash_table_[index].first.size() != 0; index = (++index) % slot_max_size_) {
            if (my_hash_table_[index].first == key) {
                my_hash_table_[index].second = value;
                return;
            }
        }
        if (is_current_in_memory_table_fulll_ == false) {
            my_hash_table_[index].first = key;
            my_hash_table_[index].second = value;
            if (current_size_ / slot_max_size_ > 0.8) {
                if (slot_max_size_ * 2 < data_set_alignment_info_ptr_->hash_in_memory_tuple_size_)
                    rebuild();
                else {
                    is_current_in_memory_table_fulll_ = true;
                }
                ++current_size_;
            }
        } else {
            //write to the db file
        }
    }
};


class Answer {
private:

    yche_string_string_map<> yche_map_;
    bool is_file_exists_{false};
    char *one_alignment_content_buffer_;
    fstream db_file_stream_;
    size_t count{0};
    DataSetAlignmentInfo *data_set_alignment_info_ptr_{nullptr};

    void inline init_data_set_alignment_info(DataSetType data_set_type) {
        data_set_alignment_info_ptr_ = new DataSetAlignmentInfo(data_set_type);
    }

    void inline create_file(char *filename) {
        db_file_stream_.open(filename, ios::out | ios::trunc);
        db_file_stream_.close();
        db_file_stream_.open(filename, ios::in | ios::out);
    }

    void inline init_yche_hash_map() {
        yche_map_.db_file_stream_ptr_ = &db_file_stream_;
        yche_map_.data_set_alignment_info_ptr_ = data_set_alignment_info_ptr_;
    }

    void inline read_portion_of_file_to_hash_table(char *filename) {

    }

public:
    Answer() {
        init_yche_hash_map();
        db_file_stream_.open(SMALL_FILE_NAME, ios::in | ios::out);
        is_file_exists_ = true;
        if (db_file_stream_.good()) {
            read_portion_of_file_to_hash_table(SMALL_FILE_NAME);
            init_data_set_alignment_info(DataSetType::small);
        } else {
            db_file_stream_.open(MEDIUM_FILE_NAME, ios::in | ios::out);
            if (db_file_stream_.good()) {
                read_portion_of_file_to_hash_table(MEDIUM_FILE_NAME);
                init_data_set_alignment_info(DataSetType::medium);
            }
            else {
                db_file_stream_.open(MEDIUM_FILE_NAME, ios::in | ios::out);
                if (db_file_stream_.good()) {
                    read_portion_of_file_to_hash_table(LARGE_FILE_NAME);
                    init_data_set_alignment_info(DataSetType::large);
                }
                else
                    is_file_exists_ = false;
            }
        }
    }

    virtual ~Answer() {
        delete[]one_alignment_content_buffer_;
        delete data_set_alignment_info_ptr_;
    }

    inline string get(string &&key) { //读取KV
        auto result = yche_map_.find(key);
        if (result != nullptr) {
            return *result;
        }
        else {
            return "NULL"; //文件不存在，说明该Key不存在，返回NULL
        }
    }

    inline void put(string &&key, string &&value) { //存储KV
        if (is_file_exists_ == false) {
            auto value_size = value.size();
            if (value_size <= static_cast<int>(ValueAlignment::small)) {
                create_file(SMALL_FILE_NAME);
                init_data_set_alignment_info(DataSetType::small);
            } else if (value_size <= static_cast<int>(ValueAlignment::medium)) {
                create_file(MEDIUM_FILE_NAME);
                init_data_set_alignment_info(DataSetType::medium);
            }
            else {
                create_file(LARGE_FILE_NAME);
                init_data_set_alignment_info(DataSetType::large);
            }

        }
        yche_map_.insert_or_replace(key, value);
        ++count;
        if (count % 9 == 1 || count % 9 == 3 || count % 9 == 5 || count % 9 == 7) {
            db_file_stream_ << flush;
        }
    }
};

#endif //KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H