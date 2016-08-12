//
// Created by cheyulin on 8/10/16.
//

#ifndef KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H
#define KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <list>
#include <cstddef>
#include <functional>

using namespace std;

#define FILE_NAME "tuple_transaction.db"
#define SEPERATOR ","
#define SEPERATOR_END_CHAR ';'
#define SEPERATOR_END_STRING ";"
#define SEPERATOR_CHAR ','

std::hash<string> str_hash_func_basic;

static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789+/";


static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}


std::string base64_encode(unsigned char const *bytes_to_encode, unsigned int in_len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];
        while ((i++ < 3))
            ret += '=';
    }
    return std::move(ret);

}

inline std::string base_64_encoding_wrapper(const std::string &string_to_encode) {
    return std::move(base64_encode(reinterpret_cast<const unsigned char *>(string_to_encode.c_str()),
                                   string_to_encode.length()));
}

auto str_hash_func = [](const string &key) {
    return str_hash_func_basic(base_64_encoding_wrapper(key));
};

template<size_t slot_num = 100000>
class yche_string_string_map {
    vector<std::list<pair<string, string>>> my_hash_table_;
    size_t current_size_{0};

public:
    yche_string_string_map() : my_hash_table_(slot_num) {}

    inline size_t size() {
        return current_size_;
    }

    inline string *find(const string &key) {
        auto index = str_hash_func_basic(key) % slot_num;
        for (auto &my_pair:my_hash_table_[index]) {
            if (my_pair.first == key) {
                return &my_pair.second;
            }
        }
        return nullptr;
    }


    inline void insert_or_replace(const string &key, const string &value) {
        auto index = str_hash_func_basic(key) % slot_num;
        for (auto &my_pair:my_hash_table_[index]) {
            if (my_pair.first == key) {
                my_pair.second = value;
                return;
            }
        }
        my_hash_table_[index].push_back(make_pair(key, value));
        ++current_size_;
    }

};

class Answer {
private:
    yche_string_string_map<50000> yche_map_;
    fstream output_file_stream_;
    size_t count{0};
    bool first_time{true};

    inline pair<string, string> split(const string &str) {
        auto iter_begin = str.begin();
        auto iter_end = str.end();
        auto iter_middle = find(iter_begin, iter_end, ',');

        return std::move(make_pair(std::move(string(iter_begin, iter_middle)),
                                   std::move(string(iter_middle + 1, iter_end - 1))));
    }

public: //put和get方法要求public
    Answer() {
        ifstream input_file_stream{FILE_NAME, ifstream::in};

        if (input_file_stream.is_open()) {
            input_file_stream.seekg(0, ios::end);
            size_t buffer_size = input_file_stream.tellg();
            input_file_stream.seekg(0, std::ios::beg);
            char *file_content = new char[buffer_size];
            input_file_stream.read(file_content, buffer_size);
            input_file_stream.close();

            stringstream str_stream(file_content);
            string tmp_string;
            for (; str_stream.good();) {
                getline(str_stream, tmp_string);
                if (tmp_string.size() > 0 && tmp_string.substr(tmp_string.size() - 1) == SEPERATOR_END_STRING) {
                    auto my_pair = std::move(split(tmp_string));
                    yche_map_.insert_or_replace(my_pair.first, my_pair.second);
                }
            }
            delete[](file_content);
        } else {
            input_file_stream.close();
        }

        if (yche_map_.size() >= 100000) {
            first_time = false;
        }
        output_file_stream_.open(FILE_NAME, std::ofstream::out | std::ofstream::app);
    }

    inline string get(string key) { //读取KV
        auto result = yche_map_.find(key);
        if (result == nullptr) {
            return "NULL"; //文件不存在，说明该Key不存在，返回NULL
        }
        else {
            return *result;
        }
    }

    inline void put(string key, string value) { //存储KV
        yche_map_.insert_or_replace(key, value);
        ++count;
        output_file_stream_ << key << SEPERATOR << value << SEPERATOR_END_CHAR << '\n';
        if (first_time) {
            if (yche_map_.size() < 100000) {
                output_file_stream_ << flush;
            }
            else {
                first_time = false;
                count = 0;
            }
        }
        else if (count > 5000) {
            output_file_stream_ << flush;
            count = 0;
        }
    }
};

#endif //KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H