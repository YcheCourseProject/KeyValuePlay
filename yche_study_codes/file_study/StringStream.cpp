//
// Created by cheyulin on 8/12/16.
//

#include <sstream>
#include <fstream>
#include <algorithm>
#include <iostream>

#define SEPERATOR_END_STRING ";"
#define FILE_NAME "tuple_transaction.db"

using namespace std;

inline pair<string, string> split(const string &str) {
    auto iter_begin = str.begin();
    auto iter_end = str.end();
    auto iter_middle = find(iter_begin, iter_end, ',');
    return std::move(make_pair(std::move(string(iter_begin, iter_middle)),
                               std::move(string(iter_middle + 1, iter_end - 1))));
}

int main() {
    ifstream input_stream{FILE_NAME, ios::in};

    input_stream.seekg(0, ios::end);
    size_t buffer_size = input_stream.tellg();
    cout << "Size:" << buffer_size << endl;
    input_stream.seekg(0, std::ios::beg);
    char *file_content = new char[buffer_size];
    input_stream.read(file_content, buffer_size);

    stringstream str_stream(file_content);
    string tmp_string;
    for (; str_stream.good();) {
        getline(str_stream, tmp_string);
        if (tmp_string.size() > 0 && tmp_string.substr(tmp_string.size() - 1) == SEPERATOR_END_STRING) {
            auto my_pair = std::move(split(tmp_string));
            cout << "First:" << my_pair.first << "Second:" << my_pair.second << endl;
        }
    }

    delete[](file_content);
    cout << "Finished" << endl;
}
