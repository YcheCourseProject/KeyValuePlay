//
// Created by cheyulin on 9/1/16.
//

#ifndef KEYVALUESTORE_BLAHGEEK_H
#define KEYVALUESTORE_BLAHGEEK_H

#define _GNU_SOURCE

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <assert.h>
#include <sys/uio.h>
#include <vector>
#include <string>
#include <utility>

using std::string;

// #define MAX_MEM_SIZE (256 * 1024 * 1024)
#define MAX_VALUE_LENGTH (32 * 1024)

#define HASH_ENTRY (1 * 1024 * 1024)
#define HASH_PREFIX_LEN (8)

#define METAFILE_EXPAND_SIZE (32 * 1024 * 1024)
#define VALUEFILE_INITIAL_SIZE (512 * 1024 * 1024)

#define MAX_INLINE_VALUE_SIZE (28 * 1024)
#define MAX_INLINE_MEM_SIZE (288 * 1024 * 1024)

#define    FORCE_INLINE inline __attribute__((always_inline))

asm(
".text\n"
        ".p2align 4,,15\n"
        ".globl    MurmurHash2\n"
        ".type    MurmurHash2, @function\n"
        "MurmurHash2:\n"
        ".cfi_startproc\n"
        "cmpl    $3, %esi\n"
        "movl    %esi, %eax\n"
        "jle    .L2_mmh\n"
        "leal    -4(%rsi), %r9d\n"
        "movl    %r9d, %r8d\n"
        "shrl    $2, %r8d\n"
        "movl    %r8d, %eax\n"
        "leaq    4(%rdi,%rax,4), %rcx\n"
        ".p2align 4,,10\n"
        ".p2align 3\n"
        ".L3_mmh:\n"
        "imull    $1540483477, (%rdi), %eax\n"
        "addq    $4, %rdi\n"
        "movl    %eax, %edx\n"
        "shrl    $24, %edx\n"
        "xorl    %edx, %eax\n"
        "imull    $1540483477, %esi, %edx\n"
        "imull    $1540483477, %eax, %esi\n"
        "xorl    %edx, %esi\n"
        "cmpq    %rcx, %rdi\n"
        "jne    .L3_mmh\n"
        "negl    %r8d\n"
        "leal    (%r9,%r8,4), %eax\n"
        ".L2_mmh:\n"
        "cmpl    $2, %eax\n"
        "je    .L5_mmh\n"
        "cmpl    $3, %eax\n"
        "je    .L6_mmh\n"
        "cmpl    $1, %eax\n"
        "je    .L7_mmh\n"
        ".L4_mmh:\n"
        "movl    %esi, %eax\n"
        "shrl    $13, %eax\n"
        "xorl    %eax, %esi\n"
        "imull    $1540483477, %esi, %eax\n"
        "movl    %eax, %esi\n"
        "shrl    $15, %esi\n"
        "xorl    %esi, %eax\n"
        "ret\n"
        ".p2align 4,,10\n"
        ".p2align 3\n"
        ".L6_mmh:\n"
        "movzbl    2(%rdi), %eax\n"
        "sall    $16, %eax\n"
        "xorl    %eax, %esi\n"
        ".L5_mmh:\n"
        "movzbl    1(%rdi), %eax\n"
        "sall    $8, %eax\n"
        "xorl    %eax, %esi\n"
        ".L7_mmh:\n"
        "movzbl    (%rdi), %eax\n"
        "xorl    %eax, %esi\n"
        "imull    $1540483477, %esi, %esi\n"
        "jmp    .L4_mmh\n"
        ".cfi_endproc\n"
);

extern "C" uint32_t MurmurHash2(const void *, int);

// for settings length of std::string without initialization
struct _string_layout {
    void *_;
    string::size_type length;
};
#define STR_SET_LEN(s, l) do{reinterpret_cast<struct _string_layout *>(&(s))->length = (l);}while(0)


static const std::string NULLSTR = std::string("NULL");

class Answer {

private:
    typedef struct {
        uint32_t meta_data_size;
        uint32_t value_data_size;
        uint32_t data_offsets[HASH_ENTRY];
    } Header;
    typedef struct {
        uint32_t key_hash;
        uint32_t key_length;
        uint32_t value_offset; // 0: inline
        uint32_t value_length;
        uint32_t next_offset;
        char value[];
    } Key;

    int meta_fd, value_fd;
    int meta_filesize, value_filesize;

    void *_data; // mmap of meta file
    Header *header; // same as _data

    string value_tmp;

public:
    Answer() {
        meta_fd = open("meta.dat", O_RDWR | O_CREAT, S_IRWXU);
        meta_filesize = lseek(meta_fd, 0, SEEK_END);
        bool initial = false;
        if (meta_filesize < METAFILE_EXPAND_SIZE) {
            meta_filesize = METAFILE_EXPAND_SIZE;
            ftruncate(meta_fd, meta_filesize);
            initial = true;
        }

        _data = mmap(NULL, meta_filesize, PROT_READ | PROT_WRITE, MAP_SHARED, meta_fd, 0);
        header = (Header *) _data;
        if (initial) {
            header->meta_data_size = sizeof(Header);
            header->value_data_size = 0;
            memset(header->data_offsets, 0, sizeof(uint32_t) * HASH_ENTRY);
        }

        value_fd = open("value.dat", O_RDWR | O_CREAT, S_IRWXU);
        value_filesize = lseek(value_fd, 0, SEEK_END);
        posix_fadvise(value_fd, 0, value_filesize, POSIX_FADV_RANDOM);

        value_tmp.reserve(MAX_VALUE_LENGTH);

        mlock(_data, meta_filesize);
        this->dedup();
    }

    void dedup_single(Key *k) {
        uint32_t *np_offset = &(k->next_offset);
        while (*np_offset != 0) {
            Key *nk = (Key *) ((char *) _data + *np_offset);
            if (k->key_length == nk->key_length && k->key_hash == nk->key_hash)
                *np_offset = nk->next_offset;
            else
                np_offset = &(nk->next_offset);
        }
    }

    void dedup_full(int n) {
        uint32_t *p_start = header->data_offsets + n;
        while (*p_start != 0) {
            Key *k = (Key *) ((char *) _data + *p_start);
            this->dedup_single(k);
            p_start = &(k->next_offset);
        }
    }

    void dedup() {
        for (int n = 0; n < HASH_ENTRY; n += 1)
            this->dedup_full(n);
    }

    string get(const string &key) {
        const char *key_p = key.c_str();
        int key_len = key.length();

        uint32_t key_hash = MurmurHash2(key_p, key_len > HASH_PREFIX_LEN ? HASH_PREFIX_LEN : key_len);
        uint32_t hash_idx = key_hash % HASH_ENTRY;
        uint32_t offset = header->data_offsets[hash_idx];

        while (offset != 0) {
            Key *k = (Key *) ((char *) _data + offset);
            if (key_hash == k->key_hash && key_len == k->key_length) {
                int value_len = k->value_length;
                char *value_p = (char *) value_tmp.c_str();

                if (k->value_offset != 0)
                    pread(value_fd, value_p, value_len, k->value_offset);
                else // inline value
                    return std::string(k->value, value_len);
                // memcpy(value_p, k->value, value_len);
                value_p[value_len] = '\0';
                STR_SET_LEN(value_tmp, value_len);

                return value_tmp;
            }

            offset = k->next_offset;
        }
        return NULLSTR;
    }

    void put(const string &key, const string &value) {
        uint32_t value_offset = 0;
        const char *value_p = value.c_str();
        int value_len = value.length();
        const char *key_p = key.c_str();
        int key_len = key.length();

        if (meta_filesize >= MAX_INLINE_MEM_SIZE || value_len >= MAX_INLINE_VALUE_SIZE) {
            // append to value_fd
            if (header->value_data_size + value_len >= value_filesize) {
                value_filesize = (value_filesize < VALUEFILE_INITIAL_SIZE) ? VALUEFILE_INITIAL_SIZE : (2 *
                                                                                                       value_filesize);
                ftruncate(value_fd, value_filesize);
                posix_fadvise(value_fd, 0, value_filesize, POSIX_FADV_RANDOM);
            }

            pwrite(value_fd, value_p, value_len, header->value_data_size);
            value_offset = header->value_data_size;
            header->value_data_size += value_len;
        }

        if (header->meta_data_size + sizeof(Key) + value_len >= meta_filesize) {
            meta_filesize += METAFILE_EXPAND_SIZE;
            ftruncate(meta_fd, meta_filesize);
            _data = mremap(_data, meta_filesize - METAFILE_EXPAND_SIZE, meta_filesize, MREMAP_MAYMOVE);
            mlock(_data, meta_filesize);
            header = (Header *) _data;
        }

        uint32_t key_hash = MurmurHash2(key_p, key_len > HASH_PREFIX_LEN ? HASH_PREFIX_LEN : key_len);
        uint32_t hash_idx = key_hash % HASH_ENTRY;
        Key *k = (Key *) ((char *) _data + header->meta_data_size);
        k->key_hash = key_hash;
        k->key_length = key_len;
        k->value_offset = value_offset;
        k->value_length = value_len;
        k->next_offset = header->data_offsets[hash_idx];
        if (value_offset == 0)
            memcpy(k->value, value_p, value_len);

        int offset = header->meta_data_size;
        header->meta_data_size += sizeof(Key) + ((value_offset == 0) ? value_len : 0);
        header->data_offsets[hash_idx] = offset;

        // this->dedup_single(hash_idx);
        // this->dedup_single(k);
    }
};

#endif //KEYVALUESTORE_BLAHGEEK_H
