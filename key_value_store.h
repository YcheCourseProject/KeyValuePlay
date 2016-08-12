//
// Created by cheyulin on 8/10/16.
//

#ifndef KeyValueStore_KEY_VALUE_STORE_H
#define KeyValueStore_KEY_VALUE_STORE_H


#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <dirent.h>
#include <algorithm>

using namespace std;

#define SUFFIX ".ychedb"

DIR *dpdf;
struct dirent *epdf;

//对于一组KV，以K作为文件名，V作为文件内容来存储
//这是一个性能相对较差的实现，瓶颈在于当Key的个数大于100万之后，Linux的文件索引就会特别慢。并且对于每个Key而言都需要打开1个文件句柄，然后写完Value之后再关掉。
class Answer {
    unordered_map<string, string> yche_map_;

public: //put和get方法要求public
    Answer() {
        //可以在构造函数中放入初始化内容，但这个方案不需要初始化
        //当由于断电等原因造成的程序重启，该构造函数会被执行，因此关于异常恢复的逻辑可以放在这里。
        dpdf = opendir(".");
        if (dpdf != NULL) {
            for (epdf = readdir(dpdf); epdf;) {
                string filename = std::move(epdf->d_name);
                if (filename.size() > 7 && filename.substr(filename.size() - 7) == SUFFIX) {
                    ifstream input_stream{filename};
                    if (input_stream.good()) {
                        string value;
                        input_stream >> value;
                        auto iter_begin = filename.begin();
                        auto iter_end = filename.end();
                        auto iter_middle = find(iter_begin, iter_end, '.');
                        string key(iter_begin, iter_middle);
                        yche_map_[std::move(key)] = std::move(value);
                    }
                    input_stream.close();
                }
                epdf = readdir(dpdf);
            }
        }
    }

    string get(string key) { //读取KV
        auto iter = yche_map_.find(key);
        if (iter != yche_map_.end()) {
            return iter->second;
        }
        else {
            return "NULL"; //文件不存在，说明该Key不存在，返回NULL
        }
    }

    void put(string key, string value) { //存储KV
        ofstream os(key + SUFFIX);
        os << value; //创建一个文件，文件名为Key，文件内容为Value
        os.close();
        yche_map_[std::move(key)] = std::move(value);
    }
};

#endif //INMEMORYKEYVALUESTOREWITHPERSISTENCE_KEY_VALUE_STORE_H
