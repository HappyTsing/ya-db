#ifndef YASQL_BPT_H
#define YASQL_BPT_H

#include <map>
#include "../include/Node.h"

class BPT {

public:
    off64_t root;        // root 节点的偏移量
    off64_t nextNew;        // 下一个插入的节点偏移量
    Node *rootNode;       // 根节点
    int64_t columnNums;
    string tableName;

public:
    BPT(off64_t root,off64_t nextNew,int64_t columnNums,string tableName);
    virtual ~BPT();
    bool insert(Key_t key, Record value);
    vector<Record> search(Key_t start, Key_t end);
    Record search(Key_t key);
    void printTree(Node *pNode);
//    void serialize();
//    static BPT *deSerialize(string path,vector<string>,string columnName);

private:
    bool InsertInternalNode(Node *pInternalNode, Key_t key, Node *pRightSon);
    Node *locateLeafNode(Key_t key);
    Node *createNode(Type_t type);
    bool flush(initializer_list<Node *> nodeList);
    bool deleteNodes(initializer_list<Node *> nodeList);
    Node *getNode(off64_t self);
};


#endif //YASQL_BPT_H
