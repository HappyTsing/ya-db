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

/* B+ 树核心配置 */
#define ORDER 15                           // B+ 树的阶数 m
#define MAX_KEY (ORDER - 1)               // 关键字最多有 m - 1 个
#define MAX_CHILDREN ORDER                // 内部节点至多包含 m 个孩子
#define MAX_VALUE (ORDER - 1)             // 叶子节点最多有 m - 1个数据

/* 结点类型 */
#define INTERNAL_NODE (Type_t)0           // 中间节点
#define LEAF_NODE (Type_t)1               // 叶子节点

#define INVALID (int64_t)0                // 无效，置为 0

class Node {

public:
    /* 通用属性 */
    Type_t type;                          // 节点类型
    int64_t keyNums;                      // 当前节点已存储的 key 数量
    off64_t self;                         // 当前节点在文件中的起始偏移量
    off64_t father;                       // 当前节点的父亲在文件中的起始偏移量
    string tableName;                     // 表名，用于指明持久文件的位置。文件名即表名。
    Key_t keys[MAX_KEY];                  // 索引值

    /* 中间节点 */
    off64_t children[MAX_CHILDREN];       // 当前中间节点的所有孩子在文件中的起始偏移量

    /* 叶子节点 */
    off64_t rightBrother;                 // 当前叶子节点的右兄弟在文件中的起始偏移量
    int64_t columnNums;                   // 叶子节点数据行的列数
    Record values[MAX_VALUE];             // 当前叶子节点的存储的所有数据。5 阶的 B+ 树，可以存储 4 条记录

public:
    Node(Type_t type, off64_t selfOffset, string tableName);

    Node(Type_t type, off64_t selfOffset, int64_t columnNums, string tableName);

    virtual ~Node();

    bool leafNodeInsert(Key_t key, Record value);

    Key_t leafNodeSplit(Node *pNewLeafNode);

    bool internalNodeInsert(Key_t key, Node *pNewChildNode);

    Key_t internalNodeSplit(Node *pNewInternalNode, Key_t key);

    void printNode();

    int64_t getNodeSpaceSize();

    void serialize();

    static Node *deSerialize(off64_t offset, string tableName);

};

#endif //YASQL_NODE_H
