#ifndef YASQL_NODE_H
#define YASQL_NODE_H
#include <vector>
#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

using namespace std;
typedef vector<int64_t> Record;
typedef int64_t Key_t;
typedef int64_t Type_t;

#define ORDER 5                           // b+ 树的阶数 m
#define MAX_KEY (ORDER - 1)                  // 关键字最多有 m - 1 个
#define MAX_CHILDREN ORDER                // 内部节点至多包含 m 个孩子
#define MAX_VALUE (ORDER - 1)                //  叶子节点最多有 m - 1个数据
#define INVALID (int64_t)0
#define INTERNAL_NODE (Type_t)0
#define LEAF_NODE (Type_t)1

/* 结点类型 */
class Node {

public:
    Type_t type;
    int64_t keyNums;
    off64_t self;
    off64_t father;
    off64_t rightBrother;
    Key_t keys[MAX_KEY];
    off64_t children[MAX_CHILDREN];
    Record values[MAX_KEY];  // key 的数量和 value 的数量相同

public:
    Node(Type_t type, off64_t selfOffset);
    virtual ~Node();
    bool leafNodeInsert(Key_t key, Record value);
    Key_t leafNodeSplit(Node *pNewLeafNode);
    bool internalNodeInsert(Key_t key, Node *pNewChildNode);
    Key_t internalNodeSplit(Node *pNewInternalNode, Key_t key);
    void serialize();
    static Node *deSerialize(off64_t offset);
    void printNode();
    int64_t getNodeSpaceSize();

};

#endif //YASQL_NODE_H
