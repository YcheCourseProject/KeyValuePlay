//
// Created by cheyulin on 8/20/16.
//
#include "efficient_io_key_value_store.h"

void basic_test() {
    Answer advanced_store;
    for (auto i = 0; i < 100; i++) {
        advanced_store.put(to_string(i), to_string(i + 1));
        cout << advanced_store.get(to_string(i)) << endl;
    }
}

void get_test() {
    Answer advanced_store;
    for (auto i = 0; i < 100; i++) {
        cout << advanced_store.get(to_string(i)) << endl;
    }
}

int main() {
    Answer naive_store;

//    basic_test();
    get_test();
}