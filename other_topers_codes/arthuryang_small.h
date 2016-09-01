//
// Created by cheyulin on 9/1/16.
//

#ifndef KEYVALUESTORE_ARTHURYANG_SMALL_H
#define KEYVALUESTORE_ARTHURYANG_SMALL_H

#include <string>
#include <vector>

#define MAX_OCCUPY_NUM 50000
#define MAX_HASH (1<<16)

using namespace std;

hash<string> hash_fun;

struct node {
    size_t key_;
    string value_;
    int next_{0};

    node() {
        value_.reserve(200);
    }
};

class hash_map {
private:
    node occupy_list_[MAX_OCCUPY_NUM];
    node *hp;
    int h[MAX_HASH];
    int node_count_ = 1;
    int now;
    int k;
public:
    void put(size_t key, const string &value) {
        k = key & 0xffff;
        now = h[k];
        for (; now;) {
            if (occupy_list_[now].key_ == key) {
                occupy_list_[now].value_ = value;
                return;
            }
            now = occupy_list_[now].next_;
        }
        occupy_list_[node_count_].key_ = key;
        occupy_list_[node_count_].value_ = value;
        occupy_list_[node_count_].next_ = h[k];
        h[k] = node_count_++;
    }

    string get(size_t key) {
        now = h[key & 0xffff];
        for (; now;) {
            hp = &occupy_list_[now];
            if (hp->key_ == key) {
                return hp->value_;
            }
            now = hp->next_;
        }
        return "NULL";
    }
};


class Answer {
    FILE *fp_;
    size_t t_;
    unsigned char len_;
    hash_map map_;

public:
    Answer() {
        fp_ = fopen("dict", "a");
        FILE *fin = fopen("dict", "rb");
        string value;
        value.reserve(300);
        char s[300];
        while (fread(&len_, sizeof(len_), 1, fin)) {
            fread(&t_, sizeof(t_), 1, fin);
            fread(s, sizeof(char), len_, fin);
            s[len_] = '\0';
            string str(s);
            map_.put(t_, str);
        }
        fclose(fin);
    }

    ~Answer() {
        fclose(fp_);
    }

    string get(string key) {
        return map_.get(hash_fun(key));
    }

    void put(string key, string value) {
        t_ = hash_fun(key);
        len_ = value.size();
        map_.put(t_, value);
        fwrite(&len_, sizeof(len_), 1, fp_);
        fwrite(&t_, sizeof(size_t), 1, fp_);
        fwrite(value.c_str(), 1, len_, fp_);
    }
};

#endif //KEYVALUESTORE_ARTHURYANG_SMALL_H