//
// Created by cheyulin on 8/13/16.
//

#include "memory_cached_hash_key_value_store.h"
#include <iostream>

void simple_test() {
    Answer advanced_store;

    cout << advanced_store.get("get") << endl;
    advanced_store.put("shit", "fdsfd");
    cout << advanced_store.get("get") << endl;
    advanced_store.put("get", "haha");
    advanced_store.put("get", "aaS");
    advanced_store.put("shit", "fdsfd");
    advanced_store.put("hello", "?");
    cout << advanced_store.get("get") << endl;
    cout << advanced_store.get("get2") << endl;
    cout << advanced_store.get("get") << endl;
    cout << advanced_store.get("shit") << endl;
    cout << advanced_store.get("hello") << endl;
}

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
//    simple_test();
    basic_test();
    get_test();
}