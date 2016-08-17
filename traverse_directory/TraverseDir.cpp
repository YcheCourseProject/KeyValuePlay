//
// Created by cheyulin on 8/12/16.
//
#include <dirent.h>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

using namespace std;

DIR *dpdf;
struct dirent *epdf;

void yche_mkdir(string dir_name) {
    mkdir(dir_name.c_str(), 0755);
}

int main() {
    stringstream ss;
    for (auto i = 0; i < 255; i++)
        ss << "1";
    yche_mkdir(ss.str());
    dpdf = opendir(".");
    if (dpdf != NULL) {
        for (epdf = readdir(dpdf); epdf;) {
            std::cout << epdf->d_name << std::endl;
            epdf = readdir(dpdf);
        }
    }
}
