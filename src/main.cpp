#include <iostream>
#include "../include/BPT.h"
void testBTP(){
    std::cout << "BPT Test" << std::endl;
    BPT bpt;

//    srand((unsigned) time(NULL)); //这是一个种子，如果不要随机功能，请把此句话注释掉


    for (int i = 1; i <= 8; i++) {
        int x = i;
//        int x = rand() % 9999 + 1;

        Record record;
        record.push_back(x);
        record.push_back(x);
        record.push_back(x);
        record.push_back(x);
        bpt.insert(x, record);
    }
    bpt.printTree(bpt.root);
}

int main() {
    testBTP();
}