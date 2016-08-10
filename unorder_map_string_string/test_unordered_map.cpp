//
// Created by cheyulin on 8/10/16.
//

#include <unordered_map>
#include <iostream>
#include <assert.h>


using namespace std;

void test_string(){
    string string1="yche";
    string string2="yche";
    assert(string1==string2);
}

void test_unorder_map_basic() {
    unordered_map<string, string> yche_map;
    yche_map.emplace("yche", "haha");
    assert(yche_map.find("yche") != yche_map.end());
    string string1="yche";
    assert(yche_map["yche"]=="haha");

    assert(yche_map.find(string1) != yche_map.end());
    assert(yche_map[string1]=="haha");

}

int main() {
    test_string();
    test_unorder_map_basic();
}