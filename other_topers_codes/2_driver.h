//
// Created by cheyulin on 9/1/16.
//

#ifndef KEYVALUESTORE_DRIVER_H
#define KEYVALUESTORE_DRIVER_H

#include <cstring>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>

using namespace std;
const static string SN = "NULL";
hash<string> ha;
constexpr int BUCKET_SIZE = 65536 * 8 - 1;
struct node {
    size_t key_;
    unsigned int len_ = 0, offset_;
};

class Answer {
private:
    int dataFd, nodeFd;
    node *nodeBuf;
    unsigned int len_;
    size_t t_;
    char s[30010];
    unsigned int offset = 0;
    int pagesize = 1 << 24;
    int pagenum = 120;
    int nowpage;
    char *dataBuf[120];
    int pagelen = 16;

public:
    Answer() {
        nodeFd = open("redis.index", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        int sz = (BUCKET_SIZE + 1) * sizeof(node);
        ftruncate(nodeFd, sz);
        nodeBuf = (node *) mmap(0, sz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, nodeFd, 0);
        dataFd = open("redis.data", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        ftruncate(dataFd, pagesize * pagenum);
        for (int i = 0; i <= BUCKET_SIZE; i++) {
            if (offset > nodeBuf[i].offset_ + nodeBuf[i].len_)continue;
            offset = nodeBuf[i].offset_ + nodeBuf[i].len_;
        }
        nowpage = offset >> 24;
        for (int i = 0; i <= pagelen; i++) {
            if (nowpage - i < 0)break;
            dataBuf[nowpage - i] = (char *) mmap(0, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, dataFd,
                                                 (nowpage - i) << 24);
        }
    }

    string get(string key) {
        t_ = ha(key);
        int k = (t_ & BUCKET_SIZE + BUCKET_SIZE) & BUCKET_SIZE;
        while (nodeBuf[k].len_) {
            if (nodeBuf[k].key_ == t_) {
                int p = nodeBuf[k].offset_ >> 24;
                if (p + pagelen >= nowpage)
                    return string(dataBuf[p] + (nodeBuf[k].offset_ & (pagesize - 1)), nodeBuf[k].len_);
                pread(dataFd, s, nodeBuf[k].len_, nodeBuf[k].offset_);
                return string(s, nodeBuf[k].len_);
            }
            k = (k + 7) & BUCKET_SIZE;
        }
        return SN;
    }

    void put(string key, string value) {
        t_ = ha(key);
        len_ = value.size();
        if (((offset + len_ - 1) >> 24) != nowpage) {
            if (nowpage >= pagelen)
                munmap(dataBuf[nowpage - pagelen], pagesize);
            nowpage++;
            dataBuf[nowpage] = (char *) mmap(0, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, dataFd, nowpage << 24);
            offset = nowpage << 24;
        }
        memcpy((char *) (dataBuf[nowpage] + (offset & (pagesize - 1))), value.c_str(), len_);
        int k = (t_ & BUCKET_SIZE + BUCKET_SIZE) & BUCKET_SIZE;
        while (nodeBuf[k].len_ && nodeBuf[k].key_ != t_)k = (k + 7) & BUCKET_SIZE;
        nodeBuf[k].key_ = t_;
        nodeBuf[k].len_ = len_;
        nodeBuf[k].offset_ = offset;
        offset += len_;
    }
};

#endif //KEYVALUESTORE_DRIVER_H
