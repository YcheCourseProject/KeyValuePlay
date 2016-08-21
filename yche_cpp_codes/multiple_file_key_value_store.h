//
// Created by cheyulin on 8/10/16.
//

#ifndef KeyValueStore_KEY_VALUE_STORE_H
#define KeyValueStore_KEY_VALUE_STORE_H

#include <string>
#include <fstream>
#include <sstream>
#include <cstddef>
#include <algorithm>
#include <array>
#include <memory>

using namespace std;

#define HASH_FUNC(x) hash_func(x)
#define DB_FILE_NUM 1000

//template<typename _Tp, typename... _Args>
//inline unique_ptr<_Tp> make_unique(_Args &&... __args) {
//    return unique_ptr<_Tp>(new _Tp(std::forward<_Args>(__args)...));
//}

std::hash<string> hash_func;

inline size_t get_hash_file_index(const string &to_persistent_string) {
    return HASH_FUNC(to_persistent_string) % DB_FILE_NUM;
}

struct PairInfo {
    int index_;
    pair<string, string> key_value_pair_;
};

struct SerializationInfo {
    int data_size_;
    int slot_size_;
    vector<PairInfo> pair_info_vec_;
};

class yche_map {
private:
    vector<pair<string, string>> my_hash_table_;
    size_t current_size_{0};
    size_t slot_max_size_{0};

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

public:
    yche_map() = default;

    yche_map(size_t slot_num) : my_hash_table_(slot_num) {}

    void reset_info(SerializationInfo &serialization_info) {
        my_hash_table_.resize(serialization_info.slot_size_);
        slot_max_size_ = serialization_info.slot_size_;
        for (auto &pair_info:serialization_info.pair_info_vec_) {
            my_hash_table_[pair_info.index_] = std::move(pair_info.key_value_pair_);
        }
    }

    inline size_t size() {
        return current_size_;
    }

    inline string *find(const string &key) {
        auto index = HASH_FUNC(key) % slot_max_size_;
        for (; my_hash_table_[index].first.size() != 0; index = (++index) % slot_max_size_) {
            if (my_hash_table_[index].first == key) {
                return &my_hash_table_[index].second;
            }
        }
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
        ++current_size_;
        my_hash_table_[index].first = key;
        my_hash_table_[index].second = value;
        if (current_size_ / slot_max_size_ > 0.7) {
            rebuild();
        }
    }

    string to_string() {
        stringstream ss;
        ss << current_size_ << "\n" << slot_max_size_ << "\n";
        for (auto i = 0; i < slot_max_size_; i++) {
            ss << i << "\n" << my_hash_table_[i].first << "\n" << my_hash_table_[i].second << "\n";
        }
        return ss.str();
    }
};

class Answer {
private:
    array<unique_ptr<fstream>, DB_FILE_NUM> db_file_array_;
    yche_map yche_map_;
    SerializationInfo serialization_info_;

public:
    Answer() {
        for (auto i = 0; i < DB_FILE_NUM; i++) {
            db_file_array_[i] = make_unique<fstream>(to_string(i), ios::in | ios::out | ios::app | ios::binary);
        }
    }

    inline string get(string &&key) { //读取KV
        size_t file_hash_index = get_hash_file_index(key);
        auto &input_file_stream = *db_file_array_[file_hash_index];

        input_file_stream.seekg(0, ios::beg);
        string tmp_string;
        if (input_file_stream.good()) {
            getline(input_file_stream, tmp_string);
            serialization_info_.data_size_ = stoi(tmp_string);
            getline(input_file_stream, tmp_string);
            serialization_info_.slot_size_ = stoi(tmp_string);
            serialization_info_.pair_info_vec_.clear();
        }
        for (; input_file_stream.good();) {
            getline(input_file_stream, tmp_string);
            if (input_file_stream.good()) {
                PairInfo pair_info;
                getline(input_file_stream, tmp_string);
                pair_info.index_ = stoi(tmp_string);
                getline(input_file_stream, tmp_string);
                pair_info.key_value_pair_.first = tmp_string;
                getline(input_file_stream, tmp_string);
                pair_info.key_value_pair_.second = tmp_string;
                serialization_info_.pair_info_vec_.push_back(pair_info);
            }
        }

        yche_map_.reset_info(serialization_info_);

        input_file_stream.clear();
        auto string_ptr = yche_map_.find(key);
        if (string_ptr == nullptr)
            return "NULL";
        else
            return *string_ptr;
    }

    //Can be optimized with first read, remove duplicate and then write whole
    inline void put(string &&key, string &&value) { //存储KV
        size_t file_hash_index = get_hash_file_index(key);
        auto &input_file_stream = *db_file_array_[file_hash_index];

        input_file_stream.seekg(0, ios::beg);
        string tmp_string;
        if (input_file_stream.good()) {
            getline(input_file_stream, tmp_string);
            serialization_info_.data_size_ = std::stoi(tmp_string);
            getline(input_file_stream, tmp_string);
            serialization_info_.slot_size_ = std::stoi(tmp_string);
            serialization_info_.pair_info_vec_.clear();
        }
        for (; input_file_stream.good();) {
            getline(input_file_stream, tmp_string);
            if (input_file_stream.good()) {
                PairInfo pair_info;
                getline(input_file_stream, tmp_string);
                pair_info.index_ = std::stoi(tmp_string);
                getline(input_file_stream, tmp_string);
                pair_info.key_value_pair_.first = tmp_string;
                getline(input_file_stream, tmp_string);
                pair_info.key_value_pair_.second = tmp_string;
                serialization_info_.pair_info_vec_.push_back(pair_info);
            }
        }

        yche_map_.reset_info(serialization_info_);

        input_file_stream.clear();
        input_file_stream << yche_map_.to_string() << flush;
    }
};

#endif //INMEMORYKEYVALUESTOREWITHPERSISTENCE_KEY_VALUE_STORE_H