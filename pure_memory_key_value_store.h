//
// Created by cheyulin on 8/10/16.
//

#ifndef KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H
#define KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H

#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>

using namespace std;

class PureMemoryStore {
private:
    unordered_map<string, string> yche_map_;

public: //put和get方法要求public
    PureMemoryStore() {
        //可以在构造函数中放入初始化内容，但这个方案不需要初始化
        //当由于断电等原因造成的程序重启，该构造函数会被执行，因此关于异常恢复的逻辑可以放在这里。
    }

    string get(string key) { //读取KV
        ifstream is(key);
        if (is.good()) { //如果文件存在
            string value;
            is >> value; //读取文件内容
            is.close();
            return value; //返回
        } else {
            return "NULL"; //文件不存在，说明该Key不存在，返回NULL
        }
    }

    void put(string key, string value) { //存储KV
        ofstream os(key);
        os << value; //创建一个文件，文件名为Key，文件内容为Value
        os.close();
    }
};

#endif //KEYVALUESTORE_PURE_MEMORY_KEY_VALUE_STORE_H
