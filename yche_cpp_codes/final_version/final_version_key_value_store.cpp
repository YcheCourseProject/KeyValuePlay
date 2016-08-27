//
// Created by cheyulin on 8/23/16.
//

#include "final_version_key_value_store.h"
#include <iostream>

void basic_test() {
    Answer advanced_store;
    string build_str;
    for (auto i = 0; i < 5000; i++) {
        build_str = build_str + to_string(i);
    }
    for (auto i = 0; i < 50000; i++) {
        advanced_store.put(to_string(i), to_string(i + 1) + build_str);
        cout << advanced_store.get(to_string(i)).size() << endl;
    }
}

void get_test() {
    Answer advanced_store;
    for (auto i = 0; i < 50000; i++) {
        cout << advanced_store.get(to_string(i)).size() << endl;
    }
}

int main() {
    Answer naive_store;

    basic_test();
    get_test();
}