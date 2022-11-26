#ifndef YASQL_BPT_H
#define YASQL_BPT_H

#include <map>
#include "../include/Node.h"

class BPT {

public:
    off64_t root;        // root 节点的偏移量
    int64_t nodeNums;  // 节点总数
    off_t nextNew;        // 下一个插入的节点偏移量
    Node *rootNode;    // 根节点缓存

public:
    BPT();
    virtual ~BPT();
    bool insert(int key, Record value);
    void printTree(Node *pNode);
    void serialize();
    static BPT *deSerialize();

private:
    bool InsertInternalNode(Node *pInternalNode, Key_t key, Node *pRightSon);
    Node *locateLeafNode(Key_t key);
    Node *createNode(Type_t type);
    bool flush(initializer_list<Node *> nodeList);
    bool deleteNodes(initializer_list<Node *> nodeList);

};


#endif //YASQL_BPT_H
