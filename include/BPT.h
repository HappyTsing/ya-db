#ifndef YASQL_BPT_H
#define YASQL_BPT_H

#include "../include/Node.h"


class BPT {

public:
    BPT();

    virtual ~BPT();

    Node *root;

    bool insert(int key, Record value);  // data 是一条记录，此处先以
    void printTree(Node *pNode);

    void serializeTree();

    void deSerializeTree();

private:
    Node *locateLeafNode(key_t key);

    bool InsertInternalNode(Node *pInternalNode, key_t key, Node *pRightSon);

};


#endif //YASQL_BPT_H
