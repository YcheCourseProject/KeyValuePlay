//
// Created by cheyulin on 8/10/16.
//

#include <unordered_map>
#include <iostream>
#include <assert.h>
#include <algorithm>


using namespace std;

void test_string() {
    string string1 = "yche";
    string string2 = "yche";
    assert(string1 == string2);
}

void test_unorder_map_basic() {
    unordered_map<string, string> yche_map;
    yche_map.emplace("yche", "haha");
    assert(yche_map.find("yche") != yche_map.end());
    string string1 = "yche";
    assert(yche_map["yche"] == "haha");

    assert(yche_map.find(string1) != yche_map.end());
    assert(yche_map[string1] == "haha");

    yche_map.insert(std::move(pair<string, string>("yche2", "hello")));
    cout << yche_map["yche2"] << endl;
}

void test_substr_and_operator_override() {
    string str = "123456";
    cout << str.substr(str.size() - 1) << endl;
    assert(str.substr(str.size() - 2) == "56");
}

constexpr char SEPERATOR = ',';

pair<string, string> split(const string &str) {
    pair<string, string> result;
    auto iter_begin = str.begin();
    auto iter_end = str.end();
    auto iter_middle = find(iter_begin, iter_end, ',');
    return std::move(make_pair(std::move(string(iter_begin, iter_middle)),
                               std::move(string(iter_middle + 1, iter_end - 1))));
}

void test_split() {
    string str = "abc,def,";
    auto my_pair = std::move(split(str));
    cout << "First:" << my_pair.first << ",Second:" << my_pair.second;
}

int main() {
    test_string();
    test_unorder_map_basic();
    test_substr_and_operator_override();
    test_split();
}