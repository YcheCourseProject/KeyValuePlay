//
// Created by cheyulin on 9/1/16.
//

#ifndef KEYVALUESTORE_SANJOSE_H
#define KEYVALUESTORE_SANJOSE_H
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include<cstdio>
#include<cstdint>
#include<cstring>
#include<string>

using namespace std;
extern "C" {
#include<unistd.h>
#include<sys/mman.h>
#include<fcntl.h>
}
#define POOLSIZE 8192000
#define MAXSLOTS 500000
#define MC (260*1024*1024)
long long FILESIZE = 2317824000;

// // for small dataset
//#define POOLSIZE 368640
//#define MAXSLOTS 20800
//#define MC (16*1024*1024)
//long long FILESIZE = 25534464;

#define MCW (4*1024*1024)
#define IDXSIZE (1024*1024*4*4)
string strnul = "NULL";

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
    register uint64_t crc = 0xffffffff;
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
    register uint64_t crc = 0xffffffff;
    while (len) {
        crc = crcTbl[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
        len--;
    }
    return (uint32_t) crc ^ 0xffffffff;
}

struct rec {
    uint32_t c32;
    uint16_t klen;
    uint16_t vlen;
    uint32_t pos;
    int next;
} __attribute__((packed));

class Answer {
public:
    Answer();

    ~Answer();

    string get(string key);

    void put(char *key, char *value);

private:
    int fd;
    FILE *pF;
    uint32_t cval;
    uint32_t hval;
    uint16_t klen;
    int *idx;
    char *buf;
    char *pbuf;
    char *cch;
    char *cchf;
    int nextSlot;
    char *pool;
    rec *ppool;
    rec *prec;
    rec trec;
    uint32_t nextPos;
    uint32_t lb_cch;
    uint32_t hb_cch;

    uint32_t fold(uint32_t c32) { return (c32 & 0x003FFFFF) ^ ((c32 >> 16) & 0xFFC0); };
};

Answer::Answer() {
    crc32c_init();
    buf = new char[30 * 1024];
    char const *fname = "kv.idx";
    nextSlot = 1;               // 0 used as a flag
    nextPos = 0;
    if (access(fname, F_OK) != -1) {
        fd = open(fname, O_RDWR);
    } else {
        fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        lseek(fd, FILESIZE - 1, SEEK_SET);
        write(fd, "", 1);
    }
    pF = fdopen(fd, "rb+");
    pool = reinterpret_cast<char *>(mmap(NULL, POOLSIZE + IDXSIZE, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0));
    cchf = reinterpret_cast<char *>(mmap(NULL, MC + 30 * 1024, PROT_READ, MAP_SHARED, fd, POOLSIZE + IDXSIZE));
    lb_cch = 0, hb_cch = MCW;
    cch = reinterpret_cast<char *>(mmap(NULL, MCW + 30 * 1024, PROT_WRITE | PROT_READ, MAP_SHARED, fd,
                                        POOLSIZE + IDXSIZE + lb_cch));
    char c;
    for (int k = 0; k * 4096 < POOLSIZE + IDXSIZE; ++k)
        c = pool[k * 4096];
    for (int k = 0; k * 4096 < MC + 30 * 1024; ++k)
        c = cchf[k * 4096];
    for (int k = 0; k * 4096 < MCW + 30 * 1024; ++k)
        c = cch[k * 4096];
    idx = reinterpret_cast<int *>(pool + POOLSIZE);
    ppool = reinterpret_cast<rec *>(pool);
    prec = ppool + 1;
    while (prec->klen != 0) {
        if (prec->pos >= nextPos)
            nextPos = prec->pos + prec->vlen;
        ++nextSlot;
        ++prec;
    }
    madvise(pool, POOLSIZE + IDXSIZE, MADV_RANDOM);
    madvise(cchf, MC + 30 * 1024, MADV_RANDOM);
    madvise(cch, MCW + 30 * 1024, MADV_SEQUENTIAL);
}

Answer::~Answer() {
    delete[] buf;
    munmap(pool, POOLSIZE + IDXSIZE);
    munmap(cchf, MC + 30 * 1024);
    munmap(cch, MCW + 30 * 1024);
    close(fd);
}

string Answer::get(string key) {
    klen = key.length();
    if (klen >= 8)
        cval = crc32(key.c_str());
    else
        cval = crc32t(key.c_str(), klen);
    hval = fold(cval);
    prec = ppool + idx[hval];
    while (prec != ppool) {
        if (prec->c32 == cval && prec->klen == klen) {
            if (prec->pos < MC) {
                pbuf = cchf + prec->pos;
            } else if (prec->pos >= lb_cch && prec->pos < hb_cch) {
                pbuf = cch + prec->pos - lb_cch;
            } else {
                fseek(pF, prec->pos + POOLSIZE + IDXSIZE, SEEK_SET);
                fread(buf, 1, prec->vlen, pF);
                pbuf = buf;
            }
            return string(pbuf, prec->vlen);
        } else {
            prec = ppool + prec->next;
        }
    }
    return strnul;
}

void Answer::put(char *key, char *value) {
    trec.klen = strlen(key);
    if (trec.klen >= 8)
        trec.c32 = crc32(key);
    else
        trec.c32 = crc32t(key, trec.klen);
    hval = fold(trec.c32);
    trec.vlen = strlen(value);
    prec = ppool + idx[hval];
    while (prec != ppool) {
        if (prec->c32 == trec.c32 && prec->klen == trec.klen) {
            if (trec.vlen == prec->vlen && prec->pos < hb_cch && prec->pos >= lb_cch) {
                pbuf = cch + prec->pos - lb_cch;
                if (memcmp(pbuf, value, trec.vlen) == 0) {
                    return;
                } else {
                    memcpy(cch + prec->pos - lb_cch, value, trec.vlen);
                    // msync(cch+prec->pos-lb_cch, trec.vlen, MS_ASYNC);
                    return;
                }
            } else {
                if (nextPos >= hb_cch) {
                    munmap(cch, MCW + 30 * 1024);
                    lb_cch = (nextPos / MCW) * MCW, hb_cch = lb_cch + MCW;
                    cch = reinterpret_cast<char *>(mmap(NULL, MCW + 30 * 1024, PROT_WRITE | PROT_READ, MAP_SHARED, fd,
                                                        POOLSIZE + IDXSIZE + lb_cch));
                    madvise(cch, MCW + 30 * 1024, MADV_SEQUENTIAL);
                }
                memcpy(cch + nextPos - lb_cch, value, trec.vlen);
                // msync(cch+nextPos-lb_cch, trec.vlen, MS_ASYNC);
                prec->vlen = trec.vlen;
                prec->pos = nextPos;
                nextPos += prec->vlen;
                // msync(prec, sizeof(rec), MS_ASYNC);
                return;
            }
        } else {
            prec = ppool + prec->next;
        }
    }
    prec = ppool + nextSlot;
    prec->next = idx[hval];
    prec->c32 = trec.c32;
    prec->klen = trec.klen, prec->vlen = trec.vlen;
    prec->pos = nextPos;
    if (nextPos >= hb_cch) {
        munmap(cch, MCW + 30 * 1024);
        lb_cch = (nextPos / MCW) * MCW, hb_cch = lb_cch + MCW;
        cch = reinterpret_cast<char *>(mmap(NULL, MCW + 30 * 1024, PROT_WRITE | PROT_READ, MAP_SHARED, fd,
                                            POOLSIZE + IDXSIZE + lb_cch));
        madvise(cch, MCW + 30 * 1024, MADV_SEQUENTIAL);
    }
    memcpy(cch + nextPos - lb_cch, value, prec->vlen);
    // msync(cch+nextPos-lb_cch, prec->vlen, MS_ASYNC);
    nextPos += prec->vlen;
    idx[hval] = nextSlot;
    ++nextSlot;
    // msync(prec, sizeof(rec), MS_ASYNC);
    // msync(idx+hval, 4, MS_ASYNC);
}

#endif //KEYVALUESTORE_SANJOSE_H
