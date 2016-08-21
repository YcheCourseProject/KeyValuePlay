//
// Created by cheyulin on 8/21/16.
//


#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>

#define DB_FILE_NAME "yche.db"

using namespace std;

int main() {
    char *logical_address;
    int file_descriptor;
    struct stat status_block;
    off_t offset, page_aligned_offset;
    offset = 5000;
    page_aligned_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);

    file_descriptor = open(DB_FILE_NAME, O_RDWR | O_CREAT, 0666);
    if (file_descriptor == -1)
        cout << "fail open" << endl;
    else
        cout << "sucessfully open" << endl;
    fstat(file_descriptor, &status_block);

    cout << status_block.st_blocks << endl;
    cout << "current page_alignment:" << page_aligned_offset << endl;
}