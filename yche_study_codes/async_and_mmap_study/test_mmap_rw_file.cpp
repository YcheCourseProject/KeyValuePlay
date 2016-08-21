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
    offset = 0;
    page_aligned_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);

    file_descriptor = open(DB_FILE_NAME, O_RDWR | O_CREAT | O_APPEND, 0600);
    if (file_descriptor == -1)
        cout << "fail open" << endl;
    else
        cout << "success open" << endl;
    fstat(file_descriptor, &status_block);

    cout << status_block.st_size << endl;
    cout << "current page_alignment:" << page_aligned_offset << endl;

    off_t length = 1024 * 1024;

    logical_address = (char *) mmap(0, length + offset - page_aligned_offset, PROT_READ | PROT_WRITE, MAP_SHARED,
                                    file_descriptor, page_aligned_offset);
    if (logical_address == MAP_FAILED) {
        cout << "map failed" << endl;
    } else {
        cout << "map success" << endl;
    }

    if (ftruncate(file_descriptor, 128) == -1)
        cout << "truncated failed" << endl;
    else
        cout << "truncated success" << endl;

    for (auto i = 0; i < 128; i++) {
        cout << static_cast<int>(logical_address[i]) << endl;
    }

    for (auto i = 0; i < 1024; i++) {
        logical_address[i] = static_cast<char>(i);
    }

//    if (msync(logical_address, 128, MS_SYNC) == -1) {
//        cout << "fail" << endl;
//    } else {
//        cout << "ok" << endl;
//    }

    close(file_descriptor);
    munmap(logical_address, length + offset - page_aligned_offset);
}