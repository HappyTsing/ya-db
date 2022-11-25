#ifndef YASQL_NODE_H
#define YASQL_NODE_H

#define ORDER 5                           // b+ 树的阶数 m
#define MAX_KEY ORDER - 1                   // 关键字最多有 m - 1 个
#define MAX_CHILDREN ORDER                // 内部节点至多包含 m 个孩子
#define MAX_VALUE ORDER - 1                //  叶子节点最多有 m - 1个数据
#define INVALID 0

#include <vector>
#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

using namespace std;
typedef vector<int64_t> Record;
typedef int64_t Key_t;

/* 结点类型 */
enum NODE_TYPE {
    INTERNAL_NODE = 0,     // 内部结点
    LEAF_NODE = 1,         // 叶子结点
};

class Node {
public:
    Node(NODE_TYPE type, off64_t selfOffset);

    virtual ~Node();

    bool leafNodeInsert(Key_t key, Record value);

    Key_t leafNodeSplit(Node *pNewLeafNode);

    bool internalNodeInsert(Key_t key, Node *pNewChildNode);

    Key_t internalNodeSplit(Node *pNewInternalNode, Key_t key);

    void printNode();

    void serialize();

    static Node *deSerialize(off64_t offset);

    int64_t getNodeSpaceSize();

public:
    NODE_TYPE type;
    int64_t keyNums;
    off64_t self;
    off64_t father;
    off64_t rightBrother;
    Key_t keys[MAX_KEY];
    off64_t children[MAX_CHILDREN];
    Record values[MAX_KEY];  // key 的数量和 value 的数量相同

};

#endif //YASQL_NODE_H
