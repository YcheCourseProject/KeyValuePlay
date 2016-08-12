//
// Created by cheyulin on 8/12/16.
//

#include <dirent.h>
#include <iostream>


DIR *dpdf;
struct dirent *epdf;

int main() {
    dpdf = opendir(".");
    if (dpdf != NULL) {
        for (epdf = readdir(dpdf); epdf;) {
            std::cout << epdf->d_name << std::endl;
            epdf = readdir(dpdf);
        }
    }
}
