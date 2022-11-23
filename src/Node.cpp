//
// Created by 王乐卿 on 2022/11/23.
//

#include "../include/Node.h"
#include <iostream>
#include <math.h>

Node::Node(NODE_TYPE type){
    this->type = type;

    // 初始化，数组暂时不知道怎么处理。
    this->keyNums=0;
    this->father=NULL;

    if(type==LEAF_NODE){
        //...将一些东西置为NULL
    }
    this->rightBrother=NULL;

}
Node::~Node() = default;

/**
 * 叶子节点插入，需要插入key和value
 * @param key
 * @param value
 * @return
 */
bool Node::leafNodeInsert(key_t key,int value){

    // 如果叶子节点已满，直接返回失败
    if(keyNums >= MAX_KEY){
        return false;
    }
    int i,j;
    // 找到要插入数据位置
    for(i = 0; i < keyNums; i++){
        if(key < keys[i]){
            break;
        }
    }

    std::cout<<"leafNodeInsert: 插入的数据位置："<< i << " key: " <<key<<  std::endl;

    // 当前位置以及其后面的 key 和 value 依次后移，空出当前位置
    for(j = keyNums; j > i ; j--){
        keys[j] = keys[j-1];
        values[j] = values[j-1];
    }
    // 写入新的 key 和 value
    keys[i] = key;
    values[i] = value;
    keyNums++;
    return true;
}

/**
 * 分裂叶子节点，把本叶子结点的后一半数据剪切到指定的叶子结点中
 * 当插入时，选择先分裂再插入的策略！
 * @return
 */
key_t Node::leafNodeSplit(Node* pNewLeafNode) {

    // 如果叶子节点未满，不允许分裂，返回空指针
    if(keyNums != MAX_KEY){
        std::cout<<"当叶子节点未满，不允许分裂！"<<std::endl;
        return -1; //TODO 排除异常
    }

    int j = 0;
    // C++ 除法自动向下取整。例如 5 阶的树，MAX_KEY = 4，此时将后 2 个 key 和 value 移动到新的叶子节点中。
    //                    例如 6 阶的树，MAX_KEY = 5，此时将后 2 个 key 和 value 移动到新的叶子节点中。
    //                    此时新的叶子节点的第一个key，将作为其父节点的key。
    for(int i = ORDER/2; i < MAX_KEY; i++){
        pNewLeafNode->keys[j] = this->keys[i];
        pNewLeafNode->values[j] = this->values[i];
        j++;
    }
    pNewLeafNode->keyNums = j;
    this->keyNums -= j;
    return pNewLeafNode->keys[0];
}

/**
 * 中间节点插入，不存储value，仅插入key即可
 * @param key
 * @param value
 * @return
 */
bool Node::internalNodeInsert(key_t key, Node* pNewChildNode) {

    // 如果中间节点已满，直接返回失败
    if(keyNums >= MAX_KEY){
        return false;
    }
    int i,j;
    // 找到要插入数据位置
    for(i = 0;i < keyNums; i++){
        if(key < keys[i]){
            break;
        }
    }

    // 当前位置以及其后面的 key 依次后移，空出当前位置
    for(j = keyNums; j > i ; j--){
        keys[j] = keys[j-1];
    }

    // 默认 孩子节点数 = keyNums + 1
    for(j = keyNums + 1; j > i + 1 ; j--){
        children[j] = children[j-1];
    }

    // 写入新的 key 和 value
    keys[i] = key;
    children[i+1] = pNewChildNode;
    pNewChildNode->father = this;
    keyNums++;
    return true;
}

/**
 * 分裂中间节点
 * 分类中间节点与叶子结点并不相同，无法单纯的一分为二，因为其不仅有 keyNums 个 key，还有 keyNums + 1 个孩子节点指针。
 * 假设 5 阶的树，MAX_KEY = 4，如果直接一份为二，则每个内部节点有 2 个 key，此时共有 3*2 个孩子节点指针。
 * 显然，是错误的，因此，应该让其中一个内部节点有 1 个 key，另一个内部节点 2 个 key，此时正好 2 + 3 = 5 个孩子节点指针。
 * 而少了的那个key，应该存储到内部节点的父节点上，此处将其命名为 upperKey.
 * 存在三种情况，此处仅讨论阶数为奇数的情况，偶数也适用，只不过不直观。
 * 例如阶数为 5，此时内部节点存了：1 3 5 7
 * (1) 当新增一个 key = 4 时，此时内部节点分为两个 【1 3】 和 【5 7】，upperKey = key = 4。分裂后，将 key = 4 的新节点，作为右侧内部节点的第一个子节点。
 * (2) 当新增一个 key < 3，例如 2 时，此时内部节点分为两个 【1】 和 [5 7] ，upperKey = 3。分裂后，将 key = 3 的新节点，插入左侧内部节点(作为其子节点）
 * (3) 当新增一个 key > 5，例如 9 时，此时内部节点分为两个 【1 3】和 【7】，upperKey = 5。分裂后，将key = 4的新节点，插入右侧内部节点（作为其子节点）
 * @return
 */
key_t Node::internalNodeSplit(Node* pNewInternalNode,key_t key) {

    // 如果节点未满，不允许分裂，返回空指针
    if(keyNums != MAX_KEY){
        return -1; // TODO: throw exception
    }

    if((keys[ORDER/2 -1] < key) && (key < keys[ORDER/2])){
        std::cout<<"key 在中间！！"<<std::endl;
        // 选择key作为中间节点
        // key 移动到新节点
        // 例如 5 阶的树，MAX_KEY = 4，把后 2 个key 移到新节点
        // 例如 6 阶的树，MAX_KEY = 5，把后 2 个key，移到新节点
        int j = 0;
        for(int i = ORDER/2; i < MAX_KEY; i++){
            pNewInternalNode->keys[j] = keys[i];
            j++;
        }

        // 指针移到新节点
        // 例如 5 阶的树，MAX_CHILDREN = 5，把后 2 个 指针 移到新节点
        // 例如 6 阶的树，MAX_CHILDREN = 6，把后 2 个 指针 移到新节点
        // 此处少了一个指针，该指针位于新节点的最左侧，后续将指向新的叶子节点
        j = 1;
        for(int i = ORDER/2 + 1; i < MAX_CHILDREN; i++){
            pNewInternalNode->children[j] = children[i];
            children[i]->father = pNewInternalNode;  // 重新设置子节点的父亲
            j++;
        }
        pNewInternalNode->keyNums =keyNums - ORDER/2 - 1;
        keyNums = ORDER/2;
        return key;
    }

    // position 是数组下标，从0开始。
    int position;
    if(key < keys[ORDER/2 - 1]) {
        std::cout<<"key 在左节点！！"<<std::endl;
        // key在左节点
        // key 移动到新节点
        // 例如 5 阶的树，MAX_KEY = 4，把后 2 个key 移到新节点
        // 例如 6 阶的树，MAX_KEY = 5，把后 2 个key 移到新节点
        position = ORDER / 2 - 1;
    }else{ // key > keys[ORDER/2]
        std::cout<<"key 在右节点！！"<<std::endl;
        // key 在右节点
        // key 移动到新节点
        // 例如 5 阶的树，MAX_KEY = 4，把后 1 个key 移到新节点
        // 例如 6 阶的树，MAX_KEY = 5，把后 1 个key 移到新节点

        position = ORDER / 2;
    }
    key_t  upperKey = keys[ORDER/2];


    int j=0;
    for(int i = position + 1; i< MAX_KEY;i++){
        pNewInternalNode->keys[j] = keys[i];
        j++;
    }

    // 注意，上移的key指向的孩子，将被新的中间节点指向！
    j = 0;
    for(int i = position + 1; i < MAX_CHILDREN;i++){
        children[i]->father=pNewInternalNode;
        pNewInternalNode->children[j]=children[i];
        j++;
    }
    pNewInternalNode->keyNums = MAX_KEY - position -1;
    keyNums = position;
    return upperKey;
}

void Node::printNode() {
    if(type == LEAF_NODE){
        std::cout<<"leaf node  ";
        std::cout<<"[ ";
        for(int i = 0;i< keyNums; i++){
            std::cout<<"(" <<keys[i]<<","<< values[i]<<") ";
        }
        std::cout<<"]" <<std::endl;
    }else{
        std::cout<<"internal node  ";
        std::cout<<"[ ";
        for(int i = 0;i< keyNums; i++){
            std::cout<< keys[i] << " ";
        }
        std::cout<<"]" <<std::endl;
    }
}