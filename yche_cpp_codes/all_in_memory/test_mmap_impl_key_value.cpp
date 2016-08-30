//
// Created by cheyulin on 8/26/16.
//
#include "mmap_impl_key_value.h"
#include <iostream>

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

    basic_test();
//    get_test();
}