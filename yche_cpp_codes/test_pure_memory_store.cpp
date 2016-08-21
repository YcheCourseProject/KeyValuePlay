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
    for (auto i = 0; i < 1000; i++) {
        naive_store.put(to_string(i), to_string(i + 1));
        cout << naive_store.get(to_string(i)) << endl;
    }
    for (auto i = 0; i < 1005; i++) {
        cout << naive_store.get(to_string(i)) << endl;
    }
//    cout << "Hi" << endl;
//    yche_map<4> hello_map;
//    for (auto i = 0; i < 3; i++) {
//        cout << "add:" << i << endl;
//        hello_map.insert_or_replace(to_string(i), to_string(i + 1));
//    }
//    for (auto &pair:hello_map.my_hash_table_) {
//        cout << "Ok:" << pair.first << "," << pair.second << endl;
//    }
//    hello_map.insert_or_replace(to_string(5), to_string(5 + 1));
//    cout << "size:" << hello_map.size() << endl;
//    cout << "slot:" << hello_map.slot_max_size_ << endl;
//    for (auto &pair:hello_map.my_hash_table_) {
//        cout << pair.first << "," << pair.second << endl;
//    }
}
