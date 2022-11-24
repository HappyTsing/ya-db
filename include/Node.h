#ifndef YASQL_NODE_H
#define YASQL_NODE_H

#define ORDER 5                           // b+ 树的阶数 m
#define MAX_KEY ORDER-1                   // 关键字最多有 m - 1 个
#define MAX_CHILDREN ORDER                // 内部节点至多包含 m 个孩子
#define INVALID 0
#include <vector>
using namespace std;
typedef vector<int64_t> Record;
typedef int key_t;

/* 结点类型 */
enum NODE_TYPE {
    INTERNAL_NODE = 1,    // 内部结点
    LEAF_NODE = 2,    // 叶子结点
};

class Node {
public:
    Node(NODE_TYPE type);

    virtual ~Node();

    bool leafNodeInsert(key_t key, Record value);

    key_t leafNodeSplit(Node *pNewLeafNode);

    bool internalNodeInsert(key_t key, Node *pNewChildNode);

    key_t internalNodeSplit(Node *pNewInternalNode, key_t key);

    void printNode();


public:
    NODE_TYPE type;
    int keyNums;
    key_t keys[MAX_KEY];
    Node *children[MAX_CHILDREN];
    Node *rightBrother;
    Node *father;
    Record values[MAX_KEY];  // key 的数量和 value 的数量相同

};

#endif //YASQL_NODE_H
