//
// Created by cheyulin on 9/1/16.
//

#ifndef KEYVALUESTORE_ZJX20_H
#define KEYVALUESTORE_ZJX20_H

#include <string>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <functional>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

using namespace std;

// #define _DEBUG

#define IL __attribute__((always_inline))
#define likely(x) __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)

typedef size_t kk_t;
// TODO: tuning those numbers
const int SMALL_BUCKET_SIZE = 48000;
#define HASH(x) hash<string>()(x)
static string STR_NULL = "NULL";

// #pragma pack(1)

int open_file(const char *name, int size) {
    int fd = open(name, O_RDWR | O_CREAT | O_NOATIME, S_IRUSR | S_IWUSR);
    struct stat sb;
    fstat(fd, &sb);
    if (sb.st_size == 0) {
        ftruncate(fd, size);
    }
    return fd;
}

bool file_exists(const char *name) {
    struct stat buf;
    int result = stat(name, &buf);
    return result == 0;
}

struct IndexItem {
    kk_t key;
    uint32_t pos;
    int32_t len;
};

class Answer {
    const int POS_BITS = 26; // 64MB
    const int POS_MASK = (1 << POS_BITS) - 1;
    const int SEG_BITS = 32 - POS_BITS;
    const int SEG_MASK = (1 << SEG_BITS) - 1;
    const int SEG_SIZE = 1 << POS_BITS;
    const int INMEM_SEGS = 1 << (28 - POS_BITS); // 2^28 = 256MB
    const int INMEM_SEGS_MASK = INMEM_SEGS - 1;

    // http://www.numberempire.com/primenumbers.php
    const int SMALL_BUCKETSIZE = 67003;
    const int MEDIUM_BUCKETSIZE = 620003; // lower bound: 400009
    const int LARGE_BUCKETSIZE = 67003;
public:
    Answer() {
        _level = 0;
        if (file_exists("is_small")) {
            _level = 1;
        } else if (file_exists("is_medium")) {
            _level = 2;
        } else if (file_exists("is_large")) {
            _level = 3;
        }

        _size = max(max(SMALL_BUCKET_SIZE, MEDIUM_BUCKETSIZE), LARGE_BUCKETSIZE);
        int map_size = sizeof(IndexItem) * (_size + 1);
        int fd = open_file("index", map_size);
        _begin = (IndexItem *) mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, fd, 0);
        _end = _begin + _size;

        setvbuf(stdout, NULL, _IOFBF, 16 * 1024 * 1024);

        if (_level != 0) {
            setup_solution(_level, true);
        }
    }

    ~Answer() {}

    IL string get(const string &key) {
        // if (unlikely(_level == 0)) {
        //   return STR_NULL;
        // }

        kk_t k = HASH(key);
        IndexItem *item = _begin + (k) % _size;;
        while (true) {
            if (likely(!item->key || item->key == k)) {
                break;
            }
            if (unlikely(++item >= _end)) {
                item = _begin;
            }
        }
        if (likely(item->key)) {
            static string str;
            int len = item->len;
            str.resize(len);
            // _data.read(item->pos, item->len, (void*)str.c_str());
            uint32_t pos = item->pos;
            int seg = (pos >> POS_BITS) & SEG_MASK;
            int real_pos = pos & POS_MASK;
            if (likely(seg + INMEM_SEGS > _n)) {
                // return _map_base[seg & INMEM_SEGS_MASK] + real_pos;
                memcpy((void *) str.c_str(), _map_base[seg & INMEM_SEGS_MASK] + real_pos, len);
            } else {
                // static char buf[31*1024];
                pread(_fds[seg], (void *) str.c_str(), len, real_pos);
                // return buf;
            }
            return str;
        } else {
            return STR_NULL;
        }
    }

    IL void put(const string &key, const string &value) {
        if (unlikely(_level == 0)) {
            if (value.size() < 200) {
                _level = 1;
            } else if (value.size() < 5000) {
                _level = 2;
            } else {
                _level = 3;
            }
            setup_solution(_level, false);
        }

        int32_t len = value.size();

        kk_t k = HASH(key);
        IndexItem *item = _begin + (k) % _size;;
        while (true) {
            if (likely(!item->key || item->key == k)) {
                break;
            }
            if (unlikely(++item >= _end)) {
                item = _begin;
            }
        }

        if (item->key) {
            uint32_t pos = item->pos;
            int seg = (pos >> POS_BITS) & SEG_MASK;
            int real_pos = pos & POS_MASK;
            int aligned_len = (item->len + (sizeof(intptr_t) - 1)) & (~(sizeof(intptr_t) - 1));
            int v_len = value.size();
            if (seg + INMEM_SEGS > _n && aligned_len >= v_len) {
                memcpy(_map_base[seg & INMEM_SEGS_MASK] + real_pos, value.c_str(), v_len);
                item->len = v_len;
                return;
            }
        }

        // _data.write(value.c_str(), len)
        if (unlikely(_pos + len > SEG_SIZE)) {
            // msync(_map_base[_n&INMEM_SEGS_MASK], SEG_SIZE, MS_ASYNC);
            int n = ++_n;
            int slot = n & INMEM_SEGS_MASK;
            if (likely(_map_base[slot] != NULL)) {
                munmap(_map_base[slot], SEG_SIZE);
            }
            static char name[10] = "data.";
            name[5] = '0' + n / 10;
            name[6] = '0' + n % 10;
            _fds[n] = open_file(name, SEG_SIZE);
            char *base = (char *) mmap(NULL, SEG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, _fds[n], 0);
            _map_base[slot] = base;
            _curr_write = base;
            _pos = 0;
            _nseg_mask = _n << POS_BITS;
        }
        uint32_t curr = _pos | _nseg_mask;
        memcpy(_curr_write, value.c_str(), len);
        int aligned_len = (len + (sizeof(intptr_t) - 1)) & (~(sizeof(intptr_t) - 1));
        _pos += aligned_len;
        _curr_write += aligned_len;

        item->key = k;
        item->pos = curr;
        item->len = len;
    }

private:
    uint32_t prefetch() {
        uint32_t eof = 0;
        for (IndexItem *it = _begin; it != _end; ++it) {
            if (it->key) {
                eof = max(eof, it->pos + it->len);
            }
        }
        return eof;
    }

    void setup_solution(int level, bool initialzing) {
        if (level == 1) {
            open("is_small", O_RDWR | O_CREAT | O_NOATIME, S_IRUSR | S_IWUSR);
            _size = SMALL_BUCKETSIZE;
            _end = _begin + _size;
        } else if (level == 2) {
            open("is_medium", O_RDWR | O_CREAT | O_NOATIME, S_IRUSR | S_IWUSR);
            _size = MEDIUM_BUCKETSIZE;
            _end = _begin + _size;
        } else {
            open("is_large", O_RDWR | O_CREAT | O_NOATIME, S_IRUSR | S_IWUSR);
            _size = LARGE_BUCKETSIZE;
            _end = _begin + _size;
        }

        uint32_t data_eof = 0;
        if (initialzing) {
            data_eof = prefetch();
        }

        // _data.init(data_eof);
        assert(sizeof(_map_base) / sizeof(_map_base[0]) >= INMEM_SEGS);

        _n = (data_eof >> POS_BITS) & SEG_MASK;
        _nseg_mask = _n << POS_BITS;
        _pos = data_eof ^ _nseg_mask;
        static char name[10] = "data.";
        for (int i = 0; i <= _n; i++) {
            name[5] = '0' + i / 10;
            name[6] = '0' + i % 10;
            _fds[i] = open_file(name, SEG_SIZE);
        }
        memset(_map_base, 0, sizeof(_map_base));
        for (int i = max(_n - INMEM_SEGS + 1, 0); i <= _n; i++) {
            char *base = (char *) mmap(NULL, SEG_SIZE,
                                       PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, _fds[i], 0);
            _map_base[i & INMEM_SEGS_MASK] = base;
            _curr_write = base;
        }
        _curr_write += _pos;
    }

private:
    int _level;
    IndexItem *_begin;
    IndexItem *_end;
    int _size;

    uint32_t _pos;
    uint32_t _nseg_mask;
    char *_curr_write;
    char *_map_base[4];
    int _fds[32];
    int _n;
};

#endif //KEYVALUESTORE_ZJX20_H
