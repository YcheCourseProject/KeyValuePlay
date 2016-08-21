//
// Created by cheyulin on 8/21/16.
//

#include <cstdint>
#include <cstring>
#include <iostream>

using namespace std;

template<typename T>
void serialize(char *buffer, T integer) {
    memcpy(buffer, &integer, sizeof(T));
}

template<typename T>
T deserialize(char *buffer) {
    T integer;
    memcpy(&integer, buffer, sizeof(T));
}

int main() {
    uint32_t integer = 123456;
    char *buffer = new char[4];
    serialize(buffer, integer);
    cout << deserialize<uint32_t>(buffer) << endl;

    uint16_t uint_16_integer = 12345;
    serialize(buffer, uint_16_integer);
    cout << deserialize<uint16_t>(buffer) << endl;
}