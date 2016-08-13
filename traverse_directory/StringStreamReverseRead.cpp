//
// Created by cheyulin on 8/13/16.
//
#include <sstream>
#include <stack>
#include <iostream>

using namespace std;

int main() {
    stringstream my_stream;
    for (auto i = 0; i < 10; i++) {
        my_stream << to_string(i) << "\n";
    }

    stack<string> tmp_string_vec;
    string tmp_string;
    for (auto i = 0; i < 10; i++) {
        getline(my_stream, tmp_string);
        tmp_string_vec.push(std::move(tmp_string));
    }

    for (; !tmp_string_vec.empty();) {
        cout << tmp_string_vec.top() << endl;
        tmp_string_vec.pop();
    }
}
