//
// Created by 王乐卿 on 2022/11/22.
//

#include "../include/BPT.h"
#include <iostream>



BPT::BPT(){
    std::cout<<"This is bpt constructor"<<std::endl;
    this->root = new Node(LEAF_NODE);
    this->root->keyNums = 0;
    this->root->type= LEAF_NODE;
}
BPT::~BPT() = default;

/* 在B+树中插入数据
插入数据首先要找到理论上要插入的叶子结点，然后分三种情况：
(1) 叶子结点未满。直接在该结点中插入即可；
(2) 叶子结点已满，且无父结点(即根结点是叶子结点)，需要首先把叶子结点分裂，然后选择插入原结点或新结点，然后新生成根节点；
(3) 叶子结点已满，但其父结点未满。需要首先把叶子结点分裂，然后选择插入原结点或新结点，再修改父结点的指针；
(4) 叶子结点已满，且其父结点已满。需要首先把叶子结点分裂，然后选择插入原结点或新结点，接着把父结点分裂，再修改祖父结点的指针。
    因为祖父结点也可能满，所以可能需要一直递归到未满的祖先结点为止。
*/
bool BPT::insert(int key, int value){

    // 1. 检查是否重复插入，调用Search

    // 2. 定位叶子节点
    Node* pLeafNode = locateLeafNode(key);
    std::cout<<"定位的叶子节点的keyNums: " << pLeafNode->keyNums <<std::endl;
    // 情况一：叶子节点未满，直接插入
    if (pLeafNode->keyNums < MAX_KEY){
            // 直接插入
        std::cout<<"叶子节点未满，直接插入"<<std::endl;
        pLeafNode->leafNodeInsert(key,value);
        return true;
    }else{ // 叶子节点已满，此时必须将叶子节点分裂
        std::cout<<"叶子节点已满，分裂叶子节点"<<std::endl;
        Node* pNewLeafNode = new Node(LEAF_NODE);
        key_t splitKey = pLeafNode->leafNodeSplit(pNewLeafNode);
        Node* pOldLeafNode = pLeafNode;    // 命名统一


        // 页节点相连
        Node* pOldLeafNodeRightBrother = pOldLeafNode->rightBrother;
        pOldLeafNode->rightBrother = pNewLeafNode;
        pNewLeafNode->rightBrother = pOldLeafNodeRightBrother;


        // 如果插入的key，小于新的叶子节点的第一个元素，则说明其应该插入到旧的叶子节点
        if(key < splitKey){
            pOldLeafNode->leafNodeInsert(key,value); // 插入原节点
        } else{
            pNewLeafNode->leafNodeInsert(key,value); // 插入新节点
        }
        Node* pFatherInternalNode = pLeafNode->father;

        // 情况二：如果原节点是根节点
        if(pFatherInternalNode == NULL){
            std::cout<<"原节点是根节点"<<std::endl;
            Node* pNewRootNode = new Node(INTERNAL_NODE);

            pNewRootNode->keys[0] = splitKey;
            pNewRootNode->children[0] = pOldLeafNode;
            pNewRootNode->children[1] = pNewLeafNode;
            pOldLeafNode->father = pNewRootNode;
            pNewLeafNode->father = pNewRootNode;
            pNewRootNode->keyNums=1;
            root = pNewRootNode;
            return true;
        }else{
            InsertInternalNode(pFatherInternalNode,splitKey,pNewLeafNode);
        }

    }

}

bool BPT::InsertInternalNode(Node* pInternalNode,key_t key,Node* pRightSon){

    if(pInternalNode == NULL || pInternalNode->type == LEAF_NODE){
        return false;
    }

    // 可以直接插入
    if(pInternalNode->keyNums < MAX_KEY){
        std::cout<<"情况三A: 内部节点未满，直接插入"<<std::endl;
        pInternalNode->internalNodeInsert(key,pRightSon);
        return true;
    }else{
        std::cout<<"情况三B: 内部节点已满，分裂内部节点"<<std::endl;
        // 节点已满，无法直接插入，此时需要对节点进行分裂
        Node* pNewInternalNode = new Node(INTERNAL_NODE);
        Node* pOldInternalNode = pInternalNode; // 命名统一
        key_t upperKey = pOldInternalNode->internalNodeSplit(pNewInternalNode,key);
        std::cout<<"情况三B: upperKey: "<< upperKey <<std::endl;

        if (key > upperKey){
            // 插入的key将在右节点
            pNewInternalNode->internalNodeInsert(key,pRightSon);
        }else if(key < upperKey){
            // 插入的key将在左节点
            pOldInternalNode->internalNodeInsert(key,pRightSon);
        }else{
            pNewInternalNode->children[0] = pRightSon;
            pRightSon->father = pInternalNode;
        }
        Node* pFatherInternalNode = pInternalNode->father;
        if(pFatherInternalNode == NULL){
            std::cout<<"情况三B: 内部节点不存在父节点"<<std::endl;
            Node* pNewRootNode = new Node(INTERNAL_NODE);
            pNewRootNode->keys[0] = upperKey;
            pNewRootNode->children[0] = pOldInternalNode;
            pNewRootNode->children[1] = pNewInternalNode;
            pOldInternalNode->father = pNewRootNode;
            pNewInternalNode->father = pNewRootNode;
            pNewRootNode->keyNums=1;
            root = pNewRootNode;
            return true;
        }else{
            return InsertInternalNode(pFatherInternalNode,upperKey,pNewInternalNode);

        }

    }
}

Node *BPT::locateLeafNode(int key) {
    // 判断根节点是索引节点还是叶子节点
    // 如果是叶子节点，则直接返回
    int i;
    Node* pNode = root;
    while(pNode!=NULL){
        if(pNode->type == LEAF_NODE){
            break;
        }
        for(i = 0; i < pNode->keyNums;i++){
            // 找到第一个键值大于key的位置，没找到，则是最右边那个子节点。
            if(key < pNode->keys[i]){
                break;
            }
        }
        pNode = pNode->children[i];
    }
    std::cout<<"定位完毕"<<std::endl;
    pNode->printNode();
    std::cout<<"打印完毕"<<std::endl;
    return pNode;
}

void BPT::printTree(Node* pNode) {
    if(pNode == NULL){
        return;
    }
    pNode->printNode();
    for(int i=0;i < pNode->keyNums +1 ;i++){
        printTree(pNode->children[i]);
    }
}