//
// Created by cheyulin on 8/22/16.
//

#ifndef KEYVALUESTORE_UTIL_H
#define KEYVALUESTORE_UTIL_H

#include <cstring>
#include <cstddef>

template<typename T>
void serialize(char *buffer, T integer) {
    memcpy(buffer, &integer, sizeof(T));
}

template<typename T>
T deserialize(char *buffer) {
    T integer;
    memcpy(&integer, buffer, sizeof(T));
}

void trim_right_blank(string &to_trim_string) {
    auto iter_back = std::find_if_not(to_trim_string.rbegin(), to_trim_string.rend(),
                                      [](int c) { return std::isspace(c); }).base();
    to_trim_string = std::string(to_trim_string.begin(), iter_back);
}

#endif //KEYVALUESTORE_UTIL_H
