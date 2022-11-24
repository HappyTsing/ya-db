#include <iostream>
#include "../include/BPT.h"
#include "../include/Table.h"
#include <fcntl.h>
void testBTP(){
    std::cout << "BPT Test" << std::endl;
    BPT bpt;

    srand((unsigned) time(NULL)); //这是一个种子，如果不要随机功能，请把此句话注释掉


    for (int i = 0; i < 200; i++) {
        int x = rand() % 9999 + 1;

        Record record;
        record.push_back(x);
        record.push_back(x-1);
        record.push_back(x-2);
        bpt.insert(x, record);
    }
    bpt.printTree(bpt.root);
}

void testTable(){
    std::cout << "Table Test" << std::endl;
    Table* table = new Table("People", {"line1", "line2", "line3"});

}

void testFcntl(){
    int fd = open("../test.txt",O_RDWR | O_CREAT);
    if(-1 == fd)
    {
        perror("open");
        printf("errno = %d\n",errno);
    }



}


int main() {
//    testBTP();
//    testTable();
    testFcntl();
}