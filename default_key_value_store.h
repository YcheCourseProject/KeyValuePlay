//
// Created by cheyulin on 8/17/16.
//

#ifndef KEYVALUESTORE_DEFAULT_KEY_VALUE_STORE_H
#define KEYVALUESTORE_DEFAULT_KEY_VALUE_STORE_H

#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <map>
#include <iomanip>
#include <algorithm>

#define INDEX_FILE_NAME "index.meta"
#define SMALL_DB_NAME "small.db"
#define MEDIUM_DB_NAME "medium.db"
#define LARGE_DB_NAME "large.db"

#define SMALL_KEY_ALIGNMENT 70
#define MEDIUM_KEY_ALIGNMENT 300
#define LARGE_KEY_ALIGNMENT 3000

#define SMALL_VALUE_ALIGNMENT 160
#define MEDIUM_VALUE_ALIGNMENT 3000
#define LARGE_VALUE_ALIGNMENT 30000

using namespace std;


class Answer {
private:
    unordered_map<string, int> key_index_map_;
    map<string, string> key_value_map_;
    fstream key_index_stream_;
    fstream db_stream_;
    int file_name_num_{0};

    int key_alignment_{0};
    int value_alignment_{0};
    int cache_max_size_{1000};
    bool is_first_in_{false};
    char *value_chars_;

    inline void read_index_info() {
        string key_str;
        string file_name_str;
        for (; key_index_stream_.good();) {
            getline(key_index_stream_, key_str);
            if (key_index_stream_.good()) {
                getline(key_index_stream_, file_name_str);
                key_index_map_[key_str] = std::stoi(file_name_str);
                file_name_num_++;
            }
        }
        key_index_stream_.clear();
    }

    inline void read_db_info() {
        key_index_stream_.open(INDEX_FILE_NAME, ios::in | ios::out | ios::app);
        if (key_index_stream_.good()) {
            db_stream_.open(SMALL_DB_NAME, ios::in | ios::out);
            if (db_stream_.good() == true) {
                key_alignment_ = SMALL_KEY_ALIGNMENT;
                value_alignment_ = SMALL_VALUE_ALIGNMENT;
            } else {
                db_stream_.open(MEDIUM_DB_NAME, ios::in | ios::out);
                if (db_stream_.good() == true) {
                    key_alignment_ = MEDIUM_KEY_ALIGNMENT;
                    value_alignment_ = MEDIUM_VALUE_ALIGNMENT;

                } else {
                    db_stream_.open(LARGE_DB_NAME, ios::in | ios::out);
                    key_alignment_ = LARGE_KEY_ALIGNMENT;
                    value_alignment_ = LARGE_VALUE_ALIGNMENT;
                }
            }
            value_chars_ = new char[value_alignment_];
        } else {
            is_first_in_ = true;
        }
    }

    inline void init_db_file(const string &value) {
        string db_file_name;
        if (value.size() <= SMALL_VALUE_ALIGNMENT) {
            db_file_name = SMALL_DB_NAME;
        } else if (value.size() <= MEDIUM_VALUE_ALIGNMENT) {
            db_file_name = MEDIUM_DB_NAME;
        } else {
            db_file_name = LARGE_DB_NAME;
        }
        db_stream_.close();
        db_stream_.open(db_file_name, ios::in | ios::out);
        value_chars_ = new char[value_alignment_];
    }

    inline void trim_right_blank(string &to_trim_string) {
        auto iter_back = find_if_not(to_trim_string.rbegin(), to_trim_string.rend(),
                                     [](int c) { return isspace(c); }).base();
        to_trim_string = string(to_trim_string.begin(), iter_back);
    }

public:
    Answer() {
        read_db_info();
        read_index_info();
    }

    virtual ~Answer() {
        delete[]value_chars_;
    }

    inline string get(string key) {
        if (key_index_map_.find(key) == key_index_map_.end()) {
            return "NULL";
        }
        else {
//            if (key_value_map_.find(key) != key_value_map_.end()) {
//                return key_value_map_[key];
//            }
            db_stream_.seekg(key_index_map_[key] * (key_alignment_ + value_alignment_) + key_alignment_);
            db_stream_.read(value_chars_, value_alignment_);
            string result_string(value_chars_);
            trim_right_blank(result_string);
            cout << "After Trim " << result_string << endl;
            return result_string;
//            ifstream is(key_file_name_map_[key]);
//            string value;
//            is >> value;
//            is.close();
        }
    }

    inline void put(string key, string value) {
        if (is_first_in_) {
            init_db_file(value);
        }
        if (key_index_map_.find(key) == key_index_map_.end()) {
            key_index_stream_ << key << "\n" << to_string(file_name_num_) << "\n" << flush;
            key_index_map_[key] = file_name_num_;
            file_name_num_++;
            db_stream_.seekp(0, ios::end);
        } else {
            db_stream_.seekp(key_index_map_[key] * (key_alignment_ + value_alignment_), ios::beg);
        }
        db_stream_ << left << setw(key_alignment_) << key << left << setw(value_alignment_) << value << flush;

//        if (key_value_map_.find(key) != key_value_map_.end() || key_value_map_.size() < cache_max_size_)
//            key_value_map_[key] = value;
//        ofstream os(key_file_name_map_[key]);
//        os << value;
//        os.close();
    }
};

#endif //KEYVALUESTORE_DEFAULT_KEY_VALUE_STORE_H
