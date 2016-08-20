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

void serialize_integer(char *buf, uint32_t val) {
    memcpy(buf, &val, 4);
}

int32_t parse_integer(const char *buf) {
    uint32_t val;
    memcpy(&val, buf, 4);
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

    char my_chars2[4];
    my_chars2[0]='1';
    my_chars2[1]='\0';
    my_chars2[2]='1';
    my_chars2[3]='\0';
    cout << my_chars2<<endl;

    string yche_str("abcd");
    cout << parse_integer(yche_str.c_str())<<endl;
}