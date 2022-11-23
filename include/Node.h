//
// Created by 王乐卿 on 2022/11/23.
//

#ifndef YASQL_NODE_H
#define YASQL_NODE_H

//#define ORDER 5                      // b+ 树的阶数 m = 5
//#define MAX_KEY 4                    // 关键字最多有 m - 1 个
//#define MAX_CHILDREN 5               // 内部节点至多包含 m 个孩子

#define ORDER 6                     // b+ 树的阶数 m = 5
#define MAX_KEY 5                    // 关键字最多有 m - 1 个
#define MAX_CHILDREN 6               // 内部节点至多包含 m 个孩子



//#define MIN_CHILDREN 3               // 内部节点至少包含 [m/2] 个孩子
//#define INVALID 0
typedef int key_t;


/* 结点类型 */
enum NODE_TYPE
{
    INTERNAL_NODE = 1,    // 内部结点
    LEAF_NODE     = 2,    // 叶子结点
};




class Node
{
public:
    Node(NODE_TYPE type);
    virtual ~Node();
    bool leafNodeInsert(key_t key, int value);
    key_t leafNodeSplit(Node* pNewLeafNode);
    bool internalNodeInsert(key_t key, Node* pNewChildNode);
    key_t internalNodeSplit(Node* pNewInternalNode,key_t key);
    void printNode();


public:
    NODE_TYPE type;
    int keyNums;
    key_t keys[MAX_KEY];
    Node* children[MAX_CHILDREN];
    Node* rightBrother;
    Node* father;
    int values[MAX_KEY];  // key 的数量和 value 的数量相同



//public:
//    NODE_TYPE getType() { return type; }
//    void setType(NODE_TYPE type){ this->type = type; }
//    int getKeyNums() { return keyNums;}
//    void setKeyNums(int keyNums) { this->keyNums = keyNums;}

};

#endif //YASQL_NODE_H
