//
// Created by cheyulin on 8/15/16.
//
#include <iostream>
#include <vector>
#include <cstring>

using namespace std;

struct key_value_info {
    string key_str_{""};
    string value_str_{""};
    int value_index_{0};
    int value_length_{0};
    int insert_count_{0};
};

int main() {
    char *my_char = "123456";
    string str(my_char, 0, 2);
    my_char = "abc";
    cout << string(my_char, 1, 1) << endl;
    cout << str << endl;
    int i = 3;
    auto index = i > 3 ? 4 : 2;
    cout << index << endl;


    vector<string> my_strs_(10);
    my_strs_.resize(5);

    for (auto str:my_strs_) {
        cout << "my string:" << endl;
    }

    vector<key_value_info> my_vec(100000);
    my_vec.resize(500);
    cout << my_vec.size() << endl;

    string tmp_str = "";
    string my_str = "yche";
    string another_str = move(my_str);
    cout << my_str.size() << endl;
    cout << tmp_str.size() << endl;

    cout << sizeof(string) << endl;

}