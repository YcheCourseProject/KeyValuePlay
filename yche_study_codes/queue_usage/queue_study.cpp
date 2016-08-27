//
// Created by cheyulin on 8/27/16.
//
#include <iostream>
#include <queue>

using namespace std;

int main() {
    queue<int> my_queue;
    for (auto i = 0; i < 5; i++) {
        my_queue.push(i);
    }

    for (; !my_queue.empty();) {
        cout << my_queue.front() << endl;
        my_queue.pop();
    }

    cout << "queue size:" << my_queue.size();
}

