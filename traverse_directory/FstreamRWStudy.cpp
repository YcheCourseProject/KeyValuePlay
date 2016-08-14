//
// Created by cheyulin on 8/14/16.
//

#include <fstream>
#include <iomanip>
#include <algorithm>
#include <iostream>

#define FLAG_ALIGNMENT 1
#define KEY_ALIGNMENT 10
#define VALUE_ALIGNMENT 10
#define YCHE_FILE "hello_fstream.txt"

using namespace std;

int main() {
    string s1 = "123";
    fstream fstream1(YCHE_FILE, ios::in | ios::out | ios::app);
    fstream1 << s1 << setw(KEY_ALIGNMENT) << "34234" << setw(VALUE_ALIGNMENT) << "\n";
    fstream1 << s1 << setw(KEY_ALIGNMENT) << "432" << setw(VALUE_ALIGNMENT) << "\n";

    fstream1.seekg(0, ios::beg);
    string tmp_string;
    for (; !fstream1.eof();) {
        getline(fstream1, tmp_string);
        cout << tmp_string << endl;
    }
}

