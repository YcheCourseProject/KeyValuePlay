//
// Created by cheyulin on 8/10/16.
//

#include "key_value_store.h"
#include <iostream>
int main() {
    Answer naive_store;

    naive_store.put("get", "haha");
    cout << naive_store.get("get") << endl;
    cout << naive_store.get("get2") << endl;
}