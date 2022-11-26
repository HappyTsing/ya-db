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

    srand((unsigned) time(NULL)); //这是一个种子，如果不要随机功能，请把此句话注释掉

    for (int i = 1; i <= 10000; i++) {
        std::cout << "======================================" << std::endl;
//        int x = rand() % 5000000 + 1;
        std::cout << "Main:testBTP: 插入第：" << i << "个数据" << std::endl;
        int x = i;
        Record record;
        // todo 目前仅支持4条
        record.push_back(x);
        record.push_back(x);
        record.push_back(x);
        record.push_back(x);
//        record.push_back(x);
//        record.push_back(x);
//        record.push_back(x);
//        record.push_back(x);
//        record.push_back(x);
//        record.push_back(x);
        bpt->insert(x, record);
//
//        std::cout << "**************打印当前树*************" << std::endl;
////        bpt->printTree(Node::deSerialize(bpt->root));
//        bpt->printTree(bpt->rootNode);
//        std::cout << "**************打印完毕*************" << std::endl;


    }
    std::cout << "======================================" << std::endl;
    std::cout << "Main:testBTP: 根节点偏移量：" << bpt->root << std::endl;
    std::cout << "Main:testBTP: 当前节点总数：" << bpt->nodeNums << std::endl;
    std::cout << "Main:testBTP: 新节点偏移量：" << bpt->nextNew << std::endl;

    bpt->printTree(bpt->rootNode);

}

//void testTable(){
//    std::cout << "Table Test" << std::endl;
//    Table* table = new Table("People", {"line1", "line2", "line3"});
//
//}

void testSerial(){
    Node* node = new Node(LEAF_NODE,1);
    node->rightBrother = 1111;
    node->father = 2222;
    node->self = 1;
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
    node->serialize();
    Node* d_node = Node::deSerialize(node->self);
    d_node->printNode();
}



int testBinary() {

    std::cout << "打开文件" << std::endl;
    int fd = open("../my.db",O_RDWR | O_CREAT,0664);
    lseek(fd,0,SEEK_SET);
    if(-1 == fd)
    {
        perror("open");
        printf("errno = %d\n",errno);
    }
    lseek(fd,0,SEEK_SET);
    std::cout << "写入" << std::endl;
//    int64_t* buffer = (int64_t*)malloc(16);
//    buffer[0] = (int64_t) 1234;
//    buffer[1] = (int64_t)5678;
//    buffer[2] = (int64_t)9011;
//    buffer[3] = (int64_t)9011;
//    buffer[4] = (int64_t)9011;
//    buffer[5] = (int64_t)9011;
//    buffer[6] = (int64_t)9011;
    int64_t* buffer = (int64_t*)malloc(100*sizeof(int64_t));
    buffer[0] = (int64_t)1;
    buffer[1] = (int64_t)2;
    buffer[2] = (int64_t)3;
    buffer[3] = (int64_t)3;
    buffer[4] = (int64_t)4;
    buffer[5] = (int64_t)6;
//    buffer = "hello world";
//    write(fd,buffer,10000);


//    int64_t keyNums = 12345678934543;
//    write(fd,&keyNums,sizeof(int64_t));
//
//    off64_t offset = 123123123123;
//    write(fd,&offset,sizeof(off64_t));
//
//    Type_t type = LEAF_NODE;
//    write(fd,&type,sizeof(Type_t));
//
//    key_t keys[3]={3};
//    write(fd,&keys,sizeof(keys));

    lseek(fd,0,SEEK_SET);
    std::cout << "读出" << std::endl;
//    int64_t buffer_read[3];
//    read(fd,buffer_read,16);
    int64_t* buf_read = (int64_t*)malloc(100*sizeof(int64_t));
    read(fd,buf_read,123);
    int64_t p = 0;
    int64_t first = buf_read[p++];
    std::cout << buf_read[0] << std::endl;

    if(first == 1){
        free(buf_read);
        buf_read = NULL;
        lseek(fd,0,SEEK_SET);
        buf_read = (int64_t*)malloc(100*sizeof(int64_t));
        read(fd,buf_read,123);
    }

    std::cout << buf_read[p++] << std::endl;
    std::cout << buf_read[p++] << std::endl;
    std::cout << buf_read[p++] << std::endl;
    std::cout << buf_read[p++] << std::endl;

    for (int i = 0; i <10; i++){
        std::cout << buf_read[4] << std::endl;
    }
    free(buf_read);
//    std::cout << buffer[0] << std::endl;
//    std::cout << buffer[1] << std::endl;
//    std::cout << buffer[2] << std::endl;
//    std::cout << buffer[3] << std::endl;
//    std::cout << buffer[4] << std::endl;
//    std::cout << buffer[5] << std::endl;
//    std::cout << buffer[6] << std::endl;


//    int64_t temp;
//    off64_t off_read;
//    Type_t type_read;

    // buffer写，直接读
//    cout << "buffer 写 直接读 " << std::endl;
//    read(fd,&temp,sizeof(int64_t));
//    std::cout << temp << std::endl;




//
//    read(fd,&temp,sizeof(int64_t));
//    read(fd,&off_read,sizeof(off64_t));
//    read(fd,&type_read,sizeof(Type_t));
//    read(fd,&keys,sizeof(keys));
//
//    std::cout << temp << std::endl;
//    std::cout << off_read << std::endl;
//    std::cout << type_read << std::endl;
//    std::cout << keys << std::endl;

}


int main() {
    testBTP();
//    testTable();
//    testSerial();
//for (int i = 0; i <210;i++){
//    testBinary();
//}
}
