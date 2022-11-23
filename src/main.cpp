#include <iostream>
#include "../include/BPT.h"
int main()
{
    std::cout<<"BPT Test"<<std::endl;
    BPT bpt;

    srand( (unsigned)time( NULL ) ); //这是一个种子，如果不要随机功能，请把此句话注释掉


    for (int i = 0; i < 30; i++){
        int x = rand()%999 +1;
        std::cout<<"======此次插入===" << x <<"=============="<<std::endl;
        bpt.insert(x,x);
        std::cout<<"==========输出成功============="<<std::endl;
    }
    bpt.printTree(bpt.root);

//    bpt.insert(1,1);
//    bpt.insert(2,2);
//    bpt.insert(3,3);
//    bpt.insert(4,4);
//    bpt.insert(5,5);
//
//    bpt.insert(6,6);
//    bpt.insert(7,7);
//    bpt.insert(8,8);
//    bpt.insert(9,9);
//    bpt.insert(10,10);
//    bpt.insert(11,10);
//    bpt.insert(12,10);
//
//    bpt.insert(13,10);
//
//    bpt.insert(14,10);
//    bpt.insert(15,10);
//    bpt.insert(16,10);
//    bpt.insert(17,10);
//    bpt.insert(18,10);
//    bpt.insert(19,10);
//    bpt.insert(20,10);
//    bpt.insert(21,10);
//    bpt.printTree(bpt.root);



}

