//
// Created by cheyulin on 8/10/16.
//
#include "pure_memory_key_value_store.h"
#include <iostream>

int main() {
    Answer naive_store;
    cout << naive_store.get("get") << endl;
    // naive_store.put("shit", "fdsfd");
    // cout << naive_store.get("get") << endl;
    naive_store.put("get", "haha");
    naive_store.put("get", "aaS");
    naive_store.put("shit", "fdsfd");
    naive_store.put("hello", "?");
    cout << naive_store.get("get") << endl;
    cout << naive_store.get("get2") << endl;
    cout << naive_store.get("get") << endl;
    cout << naive_store.get("shit") << endl;
    cout << naive_store.get("hello") << endl;
    naive_store.put("get", "haha1");
    naive_store.put("get1", "aaS1");
    naive_store.put("shit1", "fdsfd1");
    naive_store.put("hello1", "?1");
    for (auto i = 0; i < 100000000; i++) {
        naive_store.put(to_string(i), to_string(i + 1));
        cout << naive_store.get(to_string(i)) << endl;
    }
}
