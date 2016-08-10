//
// Created by cheyulin on 8/10/16.
//

#ifndef KEYVALUESTORE_SERIALIZER_H
#define KEYVALUESTORE_SERIALIZER_H

#include <string>
#include <vector>
#include <assert.h>

using namespace std;

namespace yche {
    typedef vector<uint8_t> StreamType;

    template<typename T>
    struct get_size_helper;

    template<>
    struct get_size_helper<string> {
        static size_t value(const std::string &obj) {
            return sizeof(size_t) + obj.length() * sizeof(uint8_t);
        }
    };

    template<class T>
    inline size_t get_size(const T &obj) {
        return get_size_helper<T>::value(obj);
    }

    template<class T>
    inline void serializer(const T &obj, StreamType::iterator &res) {
        serialize_helper<T>::apply(obj, res);

    }

    template<class T>
    class serialize_helper;

    template<class T>
    struct serialize_helper {
        static void apply(const T &obj, StreamType::iterator &res) {
            const uint8_t *ptr = reinterpret_cast<const uint8_t *>(&obj);
            std::copy(ptr, ptr + sizeof(T), res);
            res += sizeof(T);
        }
    };

    template<>
    struct serialize_helper<std::string> {
        static void apply(const std::string &obj, StreamType::iterator &res) {
            // store the number of elements of this string at the beginning
            serializer(obj.length(), res);
            for (const auto &cur : obj) {
                serializer(cur, res);
            }
        }
    };


    template<class T>
    inline void serialize(const T &obj, StreamType &res) {
        size_t offset = res.size();
        size_t size = get_size(obj);
        res.resize(res.size() + size);

        StreamType::iterator it = res.begin() + offset;
        serializer(obj, it);
        assert(res.begin() + offset + size == it);
    }
}

#endif //KEYVALUESTORE_SERIALIZER_H
