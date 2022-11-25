#include <iostream>
#include "../include/BPT.h"
#include "../include/Node.h"
//#include "../include/Table.h"
#include <vector>

#include <fcntl.h>
#include <unistd.h>
#include <sstream>
//enum NODE_TYPE {
//    INTERNAL_NODE = 0,     // 内部结点
//    LEAF_NODE = 1,         // 叶子结点
//};
using namespace std;
void testBTP(){
    std::cout << "BPT Test" << std::endl;
    BPT* bpt = BPT::deSerialize();
    std::cout << "Main:testBTP: 从文件获取索引树信息成功" << std::endl;
    std::cout << "Main:testBTP: 根节点偏移量：" << bpt->root << std::endl;
    std::cout << "Main:testBTP: 当前节点总数：" << bpt->nodeNums << std::endl;
    std::cout << "Main:testBTP: 新节点偏移量：" << bpt->nextNew << std::endl;

//    srand((unsigned) time(NULL)); //这是一个种子，如果不要随机功能，请把此句话注释掉
//
    for (int i = 1; i <= 15; i++) {
        std::cout << "======================================" << std::endl;
//        int x = rand() % 2000 + 1;
        std::cout << "Main:testBTP: 插入第：" << i << "个数据" << std::endl;
        int x =i;
        Record record;
        record.push_back(x);
        record.push_back(x);
        record.push_back(x);
        record.push_back(x);
        bpt->insert(x, record);

        std::cout << "**************打印当前树*************" << std::endl;
        bpt->printTree(Node::deSerialize(bpt->root));

    }
//    std::cout << "======================================" << std::endl;
//    std::cout << "Main:testBTP: 根节点偏移量：" << bpt->root << std::endl;
//    std::cout << "Main:testBTP: 当前节点总数：" << bpt->nodeNums << std::endl;
//    std::cout << "Main:testBTP: 新节点偏移量：" << bpt->nextNew << std::endl;
//
//    bpt->printTree(Node::deSerialize(bpt->root));
}

//void testTable(){
//    std::cout << "Table Test" << std::endl;
//    Table* table = new Table("People", {"line1", "line2", "line3"});
//
//}

void testSerial(){
    Node* node = new Node(LEAF_NODE,0);
    node->rightBrother = 1111;
    node->father = 2222;
    node->self = 0;
    node->keyNums=2;
    node->keys[0]=33;
    node->keys[1]=44;
    node->children[0] = 555;
    node->children[1] = 666;
    node->children[2] = 777;
    vector<int64_t> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(4);
    v.push_back(5);
    node->values[0]=v;
    node->values[1]=v;
    int64_t  size = node->getNodeSpaceSize();
    cout << size << endl;

    node->serialize();
    Node* d_node = Node::deSerialize(node->self);
    d_node->printNode();


}


int main() {
    testBTP();
//    testTable();
//    testSerial();
}


int testBinary() {
//    testBTP();
//    testTable();
//    testFcntl();

    std::cout << "打开文件" << std::endl;
    int fd = open("../my.db",O_RDWR | O_CREAT,0664);
    lseek(fd,0,SEEK_SET);
    if(-1 == fd)
    {
        perror("open");
        printf("errno = %d\n",errno);
    }

    std::cout << "写入" << std::endl;
    int64_t keyNums = 12345678934543;
    write(fd,&keyNums,sizeof(int64_t));

    off64_t offset = 123123123123;
    write(fd,&offset,sizeof(off64_t));

    NODE_TYPE type = LEAF_NODE;
    write(fd,&type,sizeof(NODE_TYPE));

    key_t keys[3]={3};
    write(fd,&keys,sizeof(keys));

    lseek(fd,0,SEEK_SET);
    std::cout << "读出" << std::endl;
    int64_t temp;
    off64_t off_read;
    NODE_TYPE type_read;

    read(fd,&temp,sizeof(int64_t));
    read(fd,&off_read,sizeof(off64_t));
    read(fd,&type_read,sizeof(NODE_TYPE));
    read(fd,&keys,sizeof(keys));

    std::cout << temp << std::endl;
    std::cout << off_read << std::endl;
    std::cout << type_read << std::endl;
    std::cout << keys << std::endl;

}
