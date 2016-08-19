//
// Created by cheyulin on 8/16/16.
//
#include <iostream>
#include <cstring>

using namespace std;

void SerializeInt32(char (&buf)[4], int32_t val) {
    std::memcpy(buf, &val, 4);
}

int32_t ParseInt32(const char (&buf)[4]) {
    int32_t val;
    std::memcpy(&val, buf, 4);
    return val;
}

int main() {
    char my_chars[4];
    int32_t my_intger = 11111111;
    SerializeInt32(my_chars, my_intger);
    cout << ParseInt32(my_chars) << endl;

    my_intger = 2222222;
    SerializeInt32(my_chars, my_intger);
    cout << ParseInt32(my_chars) << endl;
}