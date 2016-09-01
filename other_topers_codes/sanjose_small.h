//
// Created by cheyulin on 9/1/16.
//

#ifndef KEYVALUESTORE_SANJOSE_SMALL_H
#define KEYVALUESTORE_SANJOSE_SMALL_H

#include<cstdio>
#include<cstdint>
#include<cstring>
#include<string>

using namespace std;
struct [[pack]]rec {
    uint32_t c32;
    uint16_t klen;
    uint16_t vlen;
    char buf[160];
};
using array2d = rec *(*)[2];
extern "C" {
#include<unistd.h>
#include<sys/mman.h>
#include<fcntl.h>
}
string strnul = "NULL";
#define MAXSLOTS 20800
#define POLY 0x82f63b78
static uint32_t crcTbl[8][256];

static void crc32c_init(void) {
    uint32_t n, crc, k;
    for (n = 0; n < 256; n++) {
        crc = n;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crcTbl[0][n] = crc;
    }
    for (n = 0; n < 256; n++) {
        crc = crcTbl[0][n];
        for (k = 1; k < 8; k++) {
            crc = crcTbl[0][crc & 0xff] ^ (crc >> 8);
            crcTbl[k][n] = crc;
        }
    }
}

static inline uint32_t crc32(const char *buf) {
    uint64_t crc = 0xffffffff;
    crc ^= *(uint64_t *) buf;
    crc = crcTbl[7][crc & 0xff] ^
          crcTbl[6][(crc >> 8) & 0xff] ^
          crcTbl[5][(crc >> 16) & 0xff] ^
          crcTbl[4][(crc >> 24) & 0xff] ^
          crcTbl[3][(crc >> 32) & 0xff] ^
          crcTbl[2][(crc >> 40) & 0xff] ^
          crcTbl[1][(crc >> 48) & 0xff] ^
          crcTbl[0][crc >> 56];
    return (uint32_t) crc ^ 0xffffffff;
}

static inline uint32_t crc32t(const char *buf, size_t len) {
    const char *next = buf;
    uint64_t crc = 0xffffffff;
    while (len) {
        crc = crcTbl[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
        len--;
    }
    return (uint32_t) crc ^ 0xffffffff;
}

class Answer {
public:
    Answer();

    ~Answer();

    string get(string key);

    void put(char *key, char *value);

private:
    int fd;
    uint32_t cval;
    uint32_t hval;
    int klen;
    array2d idx;
    int nextSlot;
    rec *prec;
    rec trec;
    char *pool;
    rec *ppool;

    void restoreIndexFromFile();

    uint32_t fold(uint32_t c32) { return (c32 & 0x003FFFFF) ^ ((c32 >> 16) & 0xFFC0); };
};

Answer::Answer() {
    crc32c_init();
    idx = new rec *[4 * 1024 * 1024][2]();
    char const *fname = "kv.idx";
    nextSlot = 0;
    if (access(fname, F_OK) != -1) {
        fd = open(fname, O_RDWR);
        pool = reinterpret_cast<char *>(mmap(NULL, MAXSLOTS * sizeof(rec), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0));
        ppool = reinterpret_cast<rec *>(pool);
        restoreIndexFromFile();
    } else {
        fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        lseek(fd, MAXSLOTS * sizeof(rec) - 1, SEEK_SET);
        write(fd, "", 1);
        pool = reinterpret_cast<char *>(mmap(NULL, MAXSLOTS * sizeof(rec), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0));
        ppool = reinterpret_cast<rec *>(pool);
        char c;
        for (int k = 0; k * 4096 < MAXSLOTS * sizeof(rec); ++k)
            c = pool[k * 4096];
    }
}

Answer::~Answer() {
    delete[] idx;
    munmap(pool, MAXSLOTS * sizeof(rec));
    close(fd);
}

string Answer::get(string key) {
    klen = key.length();
    if (klen >= 8)
        cval = crc32(key.c_str());
    else
        cval = crc32t(key.c_str(), klen);
    hval = fold(cval);
    for (int i = 0; i < 2; ++i) {
        if (idx[hval][i] == NULL) {
            return strnul;
        } else {
            prec = idx[hval][i];
            if (prec->c32 == cval) {
                return string(prec->buf, prec->vlen);
            }
        }
    }
    return strnul;
}

void Answer::put(char *key, char *value) {
    trec.klen = strlen(key);
    trec.vlen = strlen(value);
    if (trec.klen >= 8)
        trec.c32 = crc32(key);
    else
        trec.c32 = crc32t(key, trec.klen);
    hval = fold(trec.c32);
    for (int i = 0; i < 2; ++i) {
        if (idx[hval][i] == NULL) {
            prec = ppool + nextSlot;
            prec->c32 = trec.c32;
            prec->klen = trec.klen;
            prec->vlen = trec.vlen;
            memcpy(prec->buf, value, prec->vlen);
            // msync(prec, sizeof(rec), MS_ASYNC);
            idx[hval][i] = prec;
            ++nextSlot;
            return;
        } else {
            prec = idx[hval][i];
            if (prec->c32 == trec.c32) {
                if (prec->vlen == trec.vlen && strncmp(prec->buf, value, prec->vlen) == 0) {
                    return;
                } else {
                    prec->vlen = trec.vlen;
                    memcpy(prec->buf, value, prec->vlen);
                    // msync(prec, sizeof(rec), MS_ASYNC);
                    return;
                }
            } else {
                continue;
            }
        }
    }
}

void Answer::restoreIndexFromFile() {
    prec = ppool + nextSlot;
    while (prec->klen != 0) {
        hval = fold(prec->c32);
        for (int i = 0; i < 2; ++i) {
            if (idx[hval][i] == NULL) {
                idx[hval][i] = prec;
                break;
            }
        }
        ++nextSlot;
        prec += 1;
    }
}

#endif //KEYVALUESTORE_SANJOSE_SMALL_H
