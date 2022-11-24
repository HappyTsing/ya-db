#ifndef YASQL_BPT_H
#define YASQL_BPT_H

#include "../include/Node.h"
struct BPTMeta{
    off_t root;       // root 节点的偏移量
    int64_t nodeNums; // 节点总数
};
class BPT {

public:
    BPT();
    BPTMeta meta;
    Node* rootNode; // 根节点缓存
    virtual ~BPT();

    bool insert(int key, Record value);  // data 是一条记录，此处先以
    void printTree(Node *pNode);

    void serialize(char* buffer);
    static BPTMeta deSerialize(char* buffer);

private:
    Node *locateLeafNode(key_t key);

    bool InsertInternalNode(Node *pInternalNode, key_t key, Node *pRightSon);

};


#endif //YASQL_BPT_H
