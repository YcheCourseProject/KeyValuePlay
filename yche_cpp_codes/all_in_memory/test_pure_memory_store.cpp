//
// Created by cheyulin on 8/10/16.
//
#include "pure_memory_key_value_store.h"
#include <iostream>

int main() {
    Answer naive_store;
//
    for (auto i = 0; i < 40; i++) {
        naive_store.put(to_string(i), to_string(i + 1));
        cout << naive_store.get(to_string(i)) << endl;
    }
    for (auto i = 0; i < 45; i++) {
        cout << naive_store.get(to_string(i)) << endl;
    }
}
