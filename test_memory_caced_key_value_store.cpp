//
// Created by cheyulin on 8/13/16.
//

#include "memory_cached_hash_key_value_store.h"

int main() {
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
//    advanced_store.put("get", "haha1");
//    advanced_store.put("get1", "aaS1");
//    advanced_store.put("shit1", "fdsfd1");
//    advanced_store.put("hello1", "?1");
//    for (auto i = 0; i < 100000000; i++) {
//        advanced_store.put(to_string(i), to_string(i + 1));
//        cout << advanced_store.get(to_string(i)) << endl;
//    }
}