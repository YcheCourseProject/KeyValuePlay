//
// Created by cheyulin on 8/10/16.
//

#ifndef KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H
#define KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H

#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

#define FILE_NAME "tuple_transaction.db"
#define SEPERATOR ","
#define SEPERATOR_END_CHAR ';'
#define SEPERATOR_END_STRING ";"
#define SEPERATOR_CHAR ','

class Answer {
private:
    unordered_map<string, string> yche_map_;
    list <function<void(void)>> io_function_obj_list_;
    array<thread, 1> worker_threads_;
    mutex job_queue_mutex_;
    condition_variable wait_cond_var_;
    bool is_finished_computation{false};
    bool is_io_thread_idle{false};

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

    void Task() {
        for (; !is_finished_computation;)
            next_job()();
    }

    std::function<void(void)> next_job() {
        std::function<void(void)> res;
        std::unique_lock<std::mutex> job_lock(job_queue_mutex_);
        for (; io_function_obj_list_.size() < 1;) {
            wait_cond_var_.wait(job_lock);
            is_io_thread_idle = true;
        }
        if (!is_finished_computation) {
            res = io_function_obj_list_.front();
            io_function_obj_list_.pop_front();
        }
        return res;
    }


public: //put和get方法要求public
    Answer() {
        std::ios::sync_with_stdio(false);
        yche_map_.reserve(5000000);
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
                    yche_map_[std::move(my_pair.first)] = std::move(my_pair.second);
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
        worker_threads_[0] = std::move(thread([this]() { this->Task(); }));
    }

    virtual ~Answer() {
        is_finished_computation = true;
        wait_cond_var_.notify_one();
        worker_threads_[0].join();
    }

    inline string get(string key) { //读取KV
        auto iter = yche_map_.find(key);
        if (iter != yche_map_.end()) {
            return iter->second;
        }
        else {
            return "NULL"; //文件不存在，说明该Key不存在，返回NULL
        }
    }

    inline void put(string key, string value) { //存储KV
        yche_map_[key] = value;
        ++count;
        io_function_obj_list_.emplace_back([key, value, this]() {
            this->output_file_stream_ << key << SEPERATOR << value << SEPERATOR_END_CHAR << '\n';
            if (this->first_time) {
                if (this->yche_map_.size() < 100000) {
                    output_file_stream_ << flush;
                }
                else {
                    this->first_time = false;
                    this->count = 0;
                }
            }
            else if (this->count > 5000) {
                this->output_file_stream_ << flush;
                this->count = 0;
            }
        });
        if (is_io_thread_idle == true) {
            wait_cond_var_.notify_one();
        }
    }
};

#endif //KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H