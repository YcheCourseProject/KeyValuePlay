//
// Created by cheyulin on 8/14/16.
//

#include <fstream>
#include <iomanip>
#include <algorithm>
#include <iostream>

#define FLAG_ALIGNMENT 1
#define KEY_ALIGNMENT 10
#define VALUE_ALIGNMENT 20
#define YCHE_FILE "hello_fstream.txt"

using namespace std;

void output_study() {
    fstream fstream1(YCHE_FILE, ios::in | ios::out | ios::app | ios::binary);
    string s1 = "123";
    fstream1 << "1" << left << setw(KEY_ALIGNMENT) << s1 << left << setw(KEY_ALIGNMENT) << "34234";
    fstream1.seekg(0, ios::beg);
    string tmp_string;
    for (; !fstream1.eof();) {
        getline(fstream1, tmp_string);
        cout << tmp_string << endl;
    }
}


void input_study() {
    fstream fstream1(YCHE_FILE, ios::in | ios::out | ios::app | ios::binary);
    fstream1.seekg(0, ios::end);
    int length = fstream1.tellg();
    cout << length << "!" << endl;
    fstream1.seekg(0, ios::beg);

    constexpr int alignmnet_size = 21;
    char my_char[21];

    for (auto i = 0; i < length / alignmnet_size; i++) {
        fstream1.seekg(i * alignmnet_size, ios::beg);
        fstream1.read(my_char, alignmnet_size);
        string tmp(my_char, 1);
        string tmp2(my_char, 2, 12);
        string tmp3(my_char, 12, 22);
        cout << tmp << "," << tmp2 << "," << tmp3 << endl;
    }
}

void write_frog_study() {
    fstream my_stream("frog_write.txt", ios::in | ios::out);
    if (!my_stream) {
        my_stream.open("frog_write.txt", ios::out | ios::trunc);
        my_stream.close();
        my_stream.open("frog_write.txt", ios::in | ios::out);
    }
    for (auto i = 0; i < 10; i++) {
        if (i % 2 == 1) {
            my_stream.seekp(i * 12, ios::beg);
            my_stream << left << setw(10) << i * i * i << ";\n";

        }
    }
}

void write_meta_and_read() {
    fstream my_meta_stream("yche.txt", ios::in | ios::out | ios::app);
    my_meta_stream.seekp(0, ios::beg);
    my_meta_stream << left << setw(10) << "small \n" << flush;
    my_meta_stream.seekg(0, ios::beg);
    string tmp_string;
    my_meta_stream >> tmp_string;
    cout << tmp_string;
    cout << "!" << endl;
}

int main() {
//    output_study();
//    input_study();
    write_frog_study();
//    write_meta_and_read();
}


