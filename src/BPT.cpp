#include "../include/BPT.h"
#include "../include/Utils.h"
#include <iostream>

BPT::BPT() {
    // TODO 首次运行没数据，需要特殊处理。
    BPTMeta meta = utils::readBPTMeta();
    this->meta.root = meta.root;
    this->meta.nodeNums = meta.nodeNums;
    this->rootNode = utils::readNode(this->meta.root);
}

BPT::~BPT() = default;

/**
 *
 * 插入数据首先u要找到理论上要插入的叶子结点，然后分两种情况：
 * (1) 叶子结点未满。直接在该结点中插入即可；
 * (2) 叶子结点已满，首先分裂叶子节点，然后选择将数据插入原结点或新结点；
 *     分裂后，需要将新节点的 keys[0] 插入父节点中，又分为三种情况：
 *     （2.1）无父节点，即根节点是叶子节点：生成新根节点，插入。
 *     （2.2）父节点未满：修改父节点指针，插入。
 *     （2.3）父节点已满：分裂父节点，选择插入原父节点或新父节点。
 *           由于父节点分裂后，需要将新父节点的 keys[0] 插入祖父节点中，因为祖父节点也可能满，因此需要不断递归直到未满祖先节点，或者直到满根节点，分裂该根节点，再生成新根节点。
 * @param key 索引
 * @param value 数据
 * @return 是否插入成功
 */
bool BPT::insert(int key, Record value) {

    // 1. 检查是否重复插入，调用Search

    // 2. 定位理论应该插入的叶子节点
    Node *pLeafNode = locateLeafNode(key);

    // 整棵树是空的，创建第一个节点
    if(pLeafNode == NULL){
        pLeafNode = new Node(LEAF_NODE);
        utils::writeNode(BPT_META_SIZE+this->meta.nodeNums*NODE_SIZE, pLeafNode);
    }

    // 情况（1）：叶子节点未满，直接插入
    if (pLeafNode->keyNums < MAX_KEY) {
        // 直接插入
        pLeafNode->leafNodeInsert(key, value);
        return true;
    }
        // 情况（2）：叶子节点已满，首先将叶子节点分裂，然后选择将数据插入原结点或新结点；
    else {

        // 分裂叶节点
        Node *pNewLeafNode = new Node(LEAF_NODE);
        key_t splitKey = pLeafNode->leafNodeSplit(pNewLeafNode);
        Node *pOldLeafNode = pLeafNode;    // 命名统一

        // 页节点相连
        Node *pOldLeafNodeRightBrother = pOldLeafNode->rightBrother;
        pOldLeafNode->rightBrother = pNewLeafNode;
        pNewLeafNode->rightBrother = pOldLeafNodeRightBrother;


        // 选择数据插入原节点或新节点：如果插入的key，小于新的叶子节点的第一个元素，则说明其应该插入到旧的叶子节点
        if (key < splitKey) {
            pOldLeafNode->leafNodeInsert(key, value); // 插入原节点
        } else {
            pNewLeafNode->leafNodeInsert(key, value); // 插入新节点
        }
        Node *pFatherInternalNode = pLeafNode->father;

        // 情况（2.1）：如果原节点是根节点，生成新根节点
        if (pFatherInternalNode == NULL) {
            Node *pNewRootNode = new Node(INTERNAL_NODE);
            pNewRootNode->keys[0] = splitKey;
            pNewRootNode->children[0] = pOldLeafNode;
            pNewRootNode->children[1] = pNewLeafNode;
            pOldLeafNode->father = pNewRootNode;
            pNewLeafNode->father = pNewRootNode;
            pNewRootNode->keyNums = 1;
            root = pNewRootNode;
            return true;
        } else {
            // 情况（2.1）（2.2）抽象出递归函数处理。
            InsertInternalNode(pFatherInternalNode, splitKey, pNewLeafNode);
        }
    }

}

/**
 * 递归函数，将子节点插入到其父节点
 * @param pFatherNode 将被插入的父节点
 * @param key 子节点的第一个key，即 pRightSonNode->keys[0]
 * @param pRightSonNode 插入的子节点
 * @return 是否插入成功
 */
bool BPT::InsertInternalNode(Node *pFatherNode, key_t key, Node *pRightSonNode) {

    if (pFatherNode == NULL || pFatherNode->type == LEAF_NODE) {
        return false;
    }

    // 情况（2.2）父节点未满，直接插入；
    if (pFatherNode->keyNums < MAX_KEY) {
        pFatherNode->internalNodeInsert(key, pRightSonNode);
        return true;
    }
        // 情况（2.3）父节点已满，分裂父节点
    else {
        Node *pNewInternalNode = new Node(INTERNAL_NODE);
        Node *pOldInternalNode = pFatherNode; // 命名统一
        // 分裂父节点节点进行分裂，upperKey是新节点的 keys[0]
        key_t upperKey = pOldInternalNode->internalNodeSplit(pNewInternalNode, key);
        if (key > upperKey) {
            // 将子节点插入到新父节点
            pNewInternalNode->internalNodeInsert(key, pRightSonNode);
        } else if (key < upperKey) {
            // 将子节点插入到原父节点
            pOldInternalNode->internalNodeInsert(key, pRightSonNode);
        } else {
            // 将子节点掺入到新父节点的 children[0]，即最左侧。
            pNewInternalNode->children[0] = pRightSonNode;
            pRightSonNode->father = pFatherNode;
        }

        // 由于对父节点进行了分裂，因此需要将分裂的新节点插入到祖父节点中
        Node *pGrandFatherNode = pFatherNode->father;

        // 祖父节点不存在，即父节点就是根节点，此时新建父节点
        if (pGrandFatherNode == NULL) {
            Node *pNewRootNode = new Node(INTERNAL_NODE);
            pNewRootNode->keys[0] = upperKey;
            pNewRootNode->children[0] = pOldInternalNode;
            pNewRootNode->children[1] = pNewInternalNode;
            pOldInternalNode->father = pNewRootNode;
            pNewInternalNode->father = pNewRootNode;
            pNewRootNode->keyNums = 1;
            root = pNewRootNode;
            return true;
        } else {
            // 祖父节点存在，调用该递归函数，将分裂的新内部节点插入祖父节点中。
            return InsertInternalNode(pGrandFatherNode, upperKey, pNewInternalNode);
        }

    }
}

/**
 * 基于索引定位到理论应该插入的叶子节点
 * @param key 索引
 * @return 理论应该插入的叶子节点
 */
Node *BPT::locateLeafNode(int key) {
    int i;
    Node *pNode = this->rootNode;
    while (pNode != NULL) {
        // 如果是叶子节点，则终止循环
        if (pNode->type == LEAF_NODE) {
            break;
        }
        for (i = 0; i < pNode->keyNums; i++) {
            // 找到第一个键值大于key的位置，没找到，则是最右边那个子节点。
            if (key < pNode->keys[i]) {
                break;
            }
        }
        pNode = utils::readNode(pNode->children[i]);
    }

    return pNode;
}

/**
 * 打印当前节点及其子节点，传入 root 即可打印整棵树。
 * @param pNode
 */
void BPT::printTree(Node *pNode) {
    if (pNode == NULL) {
        return;
    }
    pNode->printNode();
    for (int i = 0; i < pNode->keyNums + 1; i++) {
        printTree(pNode->children[i]);
    }
}

void BPT::serialize(char* buffer){
    stringstream ss;
    ss << '|' << this->meta.nodeNums << '|' << this->meta.root << '|';
    ss >> buffer;
}

BPTMeta BPT::deSerialize(char* buffer){
    vector<string> BPTMetaVector = utils::stringToVector(buffer,'|');
    BPTMeta d_meta;
    d_meta.nodeNums =stoll(BPTMetaVector[0]);
    d_meta.root =stoll(BPTMetaVector[1]);
    return d_meta;
}
