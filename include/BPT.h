#ifndef YASQL_BPT_H
#define YASQL_BPT_H

#include "../include/Node.h"

class BPT {

public:
    off64_t root;         // 当前 B+ 树的 root 节点在文件中的偏移量
    off64_t nextNew;      // 新建节点时，其在文件中的偏移量
    Node *rootNode;       // 根节点
    int64_t columnNums;   // 叶子节点中数据行的列数。例如用户新建的数据表有 3 列数据：{id, age, name}
    string tableName;     // 数据表的名字

public:
    BPT(off64_t root, off64_t nextNew, int64_t columnNums, string tableName);

    virtual ~BPT();

    bool insert(Key_t key, Record value);

    Record search(Key_t key);

    vector<Record> search(Key_t start, Key_t end);

    vector<Record> searchByValue(int64_t columnValueStart, int64_t columnValueEnd, int64_t columnNo);

    void printTree();

private:
    bool InsertInternalNode(Node *pInternalNode, Key_t key, Node *pRightSon);

    Node *locateLeafNode(Key_t key);

    Node *createNode(Type_t type);

    Node *getNode(off64_t self);

    void deleteNodes(initializer_list<Node *> nodeList);

    void flush(initializer_list<Node *> nodeList);

    void doPrintTree(Node *pNode);
};


#endif //YASQL_BPT_H
