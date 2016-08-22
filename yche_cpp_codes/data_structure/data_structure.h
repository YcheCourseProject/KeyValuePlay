//
// Created by cheyulin on 8/22/16.
//

#ifndef KEYVALUESTORE_DATA_STRUCTURE_H
#define KEYVALUESTORE_DATA_STRUCTURE_H

template<size_t slot_num = 200000>
class yche_map {
private:
    vector <pair<string, string>> my_hash_table_;
    size_t current_size_{0};
    size_t slot_max_size_{slot_num};

    inline void rebuild() {
        vector <pair<string, string>> rebuilding_hash_table;
        slot_max_size_ *= 2;
        rebuilding_hash_table.resize(slot_max_size_);
        for (auto previous_index = 0; previous_index < slot_max_size_ / 2; ++previous_index) {
            if (my_hash_table_[previous_index].first.size() > 0) {
                auto new_index = hash_func(my_hash_table_[previous_index].first) % slot_max_size_;
                for (; rebuilding_hash_table[new_index].first.size() != 0;
                       new_index = (++new_index) % slot_max_size_) {
                }
                rebuilding_hash_table[new_index] = move(my_hash_table_[previous_index]);
            }

        }
        my_hash_table_ = move(rebuilding_hash_table);
    }

public:
    yche_map() : my_hash_table_(slot_num) {}

    inline void reserve(int size) {
        my_hash_table_.resize(size);
    }

    inline size_t size() {
        return current_size_;
    }

    inline string *find(const string &key) {
        auto index = hash_func(key) % slot_max_size_;
        //linear probing
        for (; my_hash_table_[index].first.size() != 0; index = (index + 1) % slot_max_size_) {
            if (my_hash_table_[index].first == key) {
                return &my_hash_table_[index].second;
            }
        }
        return nullptr;
    }

    inline void insert_or_replace(const string &key, const string &value) {
        auto index = hash_func(key) % slot_max_size_;
        for (; my_hash_table_[index].first.size() != 0; index = (index + 1) % slot_max_size_) {
            if (my_hash_table_[index].first == key) {
                my_hash_table_[index].second = value;
                return;
            }
        }
        ++current_size_;
        my_hash_table_[index].first = key;
        my_hash_table_[index].second = value;
        if (current_size_ / slot_max_size_ > 0.8) {
            rebuild();
        }
    }
};

#endif //KEYVALUESTORE_DATA_STRUCTURE_H
