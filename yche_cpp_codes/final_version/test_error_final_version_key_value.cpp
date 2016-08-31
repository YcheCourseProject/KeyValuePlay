//
// Created by cheyulin on 8/27/16.
//


#include "error_final_version_key_value.h"
#include <iostream>

void basic_test() {
    Answer advanced_store;
    for (auto i = 0; i < 10; i++) {
        advanced_store.put(to_string(i), to_string(i + 1));
        cout << advanced_store.get(to_string(i)) << endl;
    }
}

void get_test() {
    Answer advanced_store;
    for (auto i = 0; i < 10; i++) {
        cout << advanced_store.get(to_string(i)) << endl;
    }
}

int main() {
    Answer naive_store;

//    basic_test();
    cout << endl << endl;
    get_test();
}