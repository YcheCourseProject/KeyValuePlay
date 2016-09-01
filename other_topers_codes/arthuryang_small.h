//
// Created by cheyulin on 9/1/16.
//

#ifndef KEYVALUESTORE_ARTHURYANG_SMALL_H
#define KEYVALUESTORE_ARTHURYANG_SMALL_H

#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <vector>

#define MAX_NUM 100100
#define MAX_HASH (1<<16)

using namespace std;

hash<string> hash_fun;
//size_t hash_fun(const string & str)
//{
//size_t hash = 5381;

//for (char c : str) {
//hash = ((hash << 5) + hash) + c; [> hash * 33 + c <]
//}

//return hash;
//}

struct node {
    size_t key;
    string value;
    int next;

    node() {
        value.reserve(200);
        next = 0;
    }
};

node hash_list[MAX_NUM];
node *hp;
int h[MAX_HASH];
int now_num = 1;
int now;
int k;

inline void hash_put(const size_t &key, const string &value) {
    k = key & 0xffff;
    now = h[k];
    while (now) {
        if (hash_list[now].key == key) {
            hash_list[now].value = value;
            return;
        }
        now = hash_list[now].next;
    }
    hash_list[now_num].key = key;
    hash_list[now_num].value = value;
    hash_list[now_num].next = h[k];
    h[k] = now_num++;
}

inline string hash_get(const size_t &key) {
    now = h[key & 0xffff];
    while (now) {
        hp = &hash_list[now];
        if (hp->key == key) {
            return hp->value;
        }
        now = hp->next;
    }
    return "NULL";
}


class Answer {
    FILE *fp;
    unordered_map<size_t, string> m;
    vector<string> v;
    unordered_map<size_t, string>::iterator p;
    string infile;
    int num;
    size_t t;
    unsigned char len;
public:
    Answer() {
        num = 0;
        fp = fopen("dict", "a");
        FILE *fin = fopen("dict", "rb");
        string value;
        value.reserve(300);
        infile.reserve(300);
        char s[300];
        while (fread(&len, sizeof(len), 1, fin)) {
            //cout << int(len) << endl;
            fread(&t, sizeof(t), 1, fin);
            fread(s, sizeof(char), len, fin);
            s[len] = '\0';
            string str(s);
            //m[t] = str;
            hash_put(t, str);
        }
        fclose(fin);
    }

    string get(const string &key) {
        return hash_get(hash_fun(key));
        //p = m.find(hash_fun(key));
        //if (p != m.end()) {
        //return p->second;
        //} else {
        //return "NULL";
        //}
    }

    void put(const string &key, const string &value) { //存储KV
        t = hash_fun(key);
        len = value.size();
        hash_put(t, value);
        fwrite(&len, sizeof(len), 1, fp);
        fwrite(&t, sizeof(size_t), 1, fp);
        fwrite(value.c_str(), 1, len, fp);

        //m[t] = value;
    }

    ~Answer() {
        fclose(fp);
    }
};


#endif //KEYVALUESTORE_ARTHURYANG_SMALL_H
