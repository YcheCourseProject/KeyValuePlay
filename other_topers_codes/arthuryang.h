//
// Created by cheyulin on 9/1/16.
//

#ifndef KEYVALUESTORE_ARTHURYANG_H
#define KEYVALUESTORE_ARTHURYANG_H

#include <stdio.h>
#include <math.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

hash<string> hash_fun;

const int MAX_SIZE = 230000000;
const int MAX_NUM = 200000;
const int MAX_HASH = 200000;
const int MAX_INIT = 300000000;
const int MAX_NUM_P = 900000;
const int MAX_HASH_P = 800000;

struct node {
    size_t key;
    string value;
    node *next;

    node() {
        next = NULL;
    }
};

node hash_list[MAX_NUM];
node *hp;
node *h[MAX_HASH];
int now_num = 1;
node *now;
int k;
int all_size = 0;

inline void hash_put(const size_t &key, const string &value) {
    k = key % MAX_HASH;
    now = h[k];
    while (now) {
        if (now->key == key) {
            now->value = value;
            return;
        }
        now = now->next;
    }
    if (all_size >= MAX_SIZE) {
        return;
    }
    now = &hash_list[now_num];
    now->key = key;
    now->value = value;
    now->next = h[k];
    h[k] = now;
    now_num++;
    all_size += value.size() - 128;
}

inline string hash_get(const size_t &key) {
    now = h[key % MAX_HASH];
    while (now) {
        if (now->key == key) {
            return now->value;
        }
        now = now->next;
    }
    return "N";
}

struct point {
    size_t key;
    unsigned short int len;
    unsigned long int all_len;
    point *next;

    point() {
        next = NULL;
    }
};

point hash_point[MAX_NUM_P];
point *p[MAX_HASH_P];
point *hpp;
int now_p = 1;

inline void hash_put_point(const size_t &key, const int &len, const int &all_len) {
    k = key % MAX_HASH_P;
    hpp = p[k];
    while (hpp) {
        if (hpp->key == key) {
            hpp->len = len;
            hpp->all_len = all_len;
            return;
        }
        hpp = hpp->next;
    }
    hpp = &hash_point[now_p];
    hpp->key = key;
    hpp->len = len;
    hpp->all_len = all_len;
    hpp->next = p[k];
    p[k] = hpp;
    now_p++;
}

inline point *hash_get_point(const size_t &key) {
    hpp = p[key % MAX_HASH_P];
    while (hpp) {
        if (hpp->key == key) {
            return hpp;
        }
        hpp = hpp->next;
    }
    return NULL;
}

class Answer {
    FILE *fp;
    FILE *fpvalue;
    FILE *fin;
    FILE *fvalue;
    string infile;
    int num;
    size_t t;
    unsigned short int len;
    string st;
    char s[30010];
    unsigned long int all_len;
public:
    Answer() {
        num = 0;
        all_len = 0;
        fp = fopen("dict", "ab");
        char fp_buff[128 * 1024];
        char fvalue_buff[128 * 1024];
        setvbuf(fp, fp_buff, _IOLBF, 122 * 1024 - 1);
        fpvalue = fopen("dict_value", "ab");
        fin = fopen("dict", "rb");
        fvalue = fopen("dict_value", "rb");
        setvbuf(fvalue, fvalue_buff, _IOLBF, 122 * 1024 - 1);
        string value;
        value.reserve(30010);
        infile.reserve(30010);
        fseek(fp, 0, SEEK_END);
        int data_number = ftell(fp) / (sizeof(len) + sizeof(t));
        fseek(fp, 0, SEEK_SET);
        int insert_data = 1000000000;
        if (data_number <= 1024) {
            insert_data = 0;
        }
        while (fread(&len, sizeof(len), 1, fin)) {
            fread(&t, sizeof(t), 1, fin);
            hash_put_point(t, len, all_len);
            if (insert_data && num == 1024) {
                insert_data = max(1024, int(data_number - MAX_INIT / (all_len / 1024.0) - 1000));
            }

            if (num == insert_data) {
                fseek(fvalue, all_len, SEEK_SET);
            }

            if (num >= insert_data) {
                fread(s, sizeof(char), len, fvalue);
                s[len] = '\0';
                string ss(s);
                hash_put(t, ss);
            }
            num += 1;
            all_len += len;
        }
    }

    string get(string key) {
        t = hash_fun(key);

        st = hash_get(t);
        if (st != "N") {
            return st;
        }

        hpp = hash_get_point(t);
        if (hpp) {
            fseek(fvalue, hpp->all_len, SEEK_SET);
            fflush(fpvalue);
            fread(s, sizeof(char), hpp->len, fvalue);
            s[hpp->len] = '\0';
            return string(s);
        } else {
            return "NULL";
        }
    }

    void put(string key, string value) {
        t = hash_fun(key);
        len = value.size();
        fwrite(value.c_str(), len, 1, fpvalue);
        fwrite(&len, sizeof(len), 1, fp);
        fwrite(&t, sizeof(t), 1, fp);
        hash_put_point(t, len, all_len);
        all_len += len;
        hash_put(t, value);
    }

    ~Answer() {
        fclose(fp);
        fclose(fin);
    }
};

#endif //KEYVALUESTORE_ARTHURYANG_H
