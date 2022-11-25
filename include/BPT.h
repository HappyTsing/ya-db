#ifndef YASQL_BPT_H
#define YASQL_BPT_H

#include <map>
#include "../include/Node.h"

class BPT {

public:
    BPT();

    off_t root;        // root 节点的偏移量
    int64_t nodeNums;  // 节点总数
    off_t nextNew;        // 下一个插入的节点偏移量
    Node *rootNode;    // 根节点缓存
    virtual ~BPT();

    bool insert(int key, Record value);  // data 是一条记录，此处先以
    void printTree(Node *pNode);

    void serialize();

    static BPT *deSerialize();

private:
    Node *locateLeafNode(Key_t key);

    Node *createNode(NODE_TYPE type);

    bool flush(initializer_list<Node *> nodeList);

    bool InsertInternalNode(Node *pInternalNode, Key_t key, Node *pRightSon);

    off_t getNewNodeOffset();


};


#endif //YASQL_BPT_H
