//
// Created by 王乐卿 on 2022/11/22.
//

#ifndef YASQL_BPT_H
#define YASQL_BPT_H

#include "../include/Node.h"



class BPT
{

public:
    BPT();
    virtual ~BPT();
    Node* root;
    bool insert(int key, int value);  // data 是一条记录，此处先以
    void printTree(Node* pNode);
    void serializeTree();
    void deSerializeTree();
    bool InsertInternalNode(Node* pInternalNode,key_t key,Node* pRightSon);

private:
    Node* locateLeafNode(key_t key);
};


#endif //YASQL_BPT_H
