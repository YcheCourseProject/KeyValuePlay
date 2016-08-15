//
// Created by cheyulin on 8/15/16.
//
#include <iostream>

using namespace std;


int main() {
    char *my_char = "123456";
    string str(my_char, 0, 2);
    cout << str<<endl;
}