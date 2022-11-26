#include "../include/BPT.h"

BPT::BPT() {}

BPT::~BPT() = default;

/**
 *
 * 插入数据首先要找到理论上要插入的叶子结点，然后分两种情况：
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

    // 情况（1）：叶子节点未满，直接插入
    if (pLeafNode->keyNums < MAX_KEY) {
        std::cout << "BPT::insert: 情况（1）：叶子节点未满，直接插入" << std::endl;
        // 直接插入
        pLeafNode->leafNodeInsert(key, value);
        flush({pLeafNode});
        deleteNodes({pLeafNode}); // 判断是否是父节点，若是，则不删除
        return true;
    }
        // 情况（2）：叶子节点已满，首先将叶子节点分裂，然后选择将数据插入原结点或新结点；
    else {
        std::cout << "BPT::insert: 情况（2）：叶子节点已满，首先将叶子节点分裂，然后选择将数据插入原结点或新结点；"
                  << std::endl;

        // 分裂叶节点
        Node *pNewLeafNode = createNode(LEAF_NODE);
        Key_t splitKey = pLeafNode->leafNodeSplit(pNewLeafNode);
        Node *pOldLeafNode = pLeafNode;    // 命名统一

        // 页节点相连
        off_t OldLeafNodeRightBrotherOffset = pOldLeafNode->rightBrother;
        pOldLeafNode->rightBrother = pNewLeafNode->self;
        pNewLeafNode->rightBrother = OldLeafNodeRightBrotherOffset;

        // 选择数据插入原节点或新节点：如果插入的key，小于新的叶子节点的第一个元素，则说明其应该插入到旧的叶子节点
        if (key < splitKey) {
            pOldLeafNode->leafNodeInsert(key, value); // 插入原节点
        } else {
            pNewLeafNode->leafNodeInsert(key, value); // 插入新节点
        }
        off_t FatherInternalNodeOffset = pLeafNode->father;

        // 情况（2.1）：如果原节点是根节点，生成新根节点
        if (FatherInternalNodeOffset == INVALID) {
            std::cout << "BPT::insert: 情况（2.1）：如果原节点是根节点，生成新根节点" << std::endl;

            Node *pNewRootNode = createNode(INTERNAL_NODE);
            pNewRootNode->keys[0] = splitKey;
            pNewRootNode->children[0] = pOldLeafNode->self;
            pNewRootNode->children[1] = pNewLeafNode->self;
            pOldLeafNode->father = pNewRootNode->self;
            pNewLeafNode->father = pNewRootNode->self;
            pNewRootNode->keyNums = 1;
            root = pNewRootNode->self;
            rootNode = pNewRootNode;
            // 更新文件
            flush({pOldLeafNode, pNewLeafNode, pNewRootNode});
            deleteNodes({pOldLeafNode, pNewLeafNode, pNewRootNode});
            return true;
        } else {
            Node *pFatherInternalNode = Node::deSerialize(FatherInternalNodeOffset);
            flush({pOldLeafNode});
            deleteNodes({pOldLeafNode});
            std::cout << "情况（2.1）（2.2）抽象出递归函数处理。" << std::endl;
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
bool BPT::InsertInternalNode(Node *pFatherNode, Key_t key, Node *pRightSonNode) {

    if (pFatherNode == NULL || pFatherNode->type == LEAF_NODE) {
        return false;
    }

    // 情况（2.2）父节点未满，直接插入；
    if (pFatherNode->keyNums < MAX_KEY) {
        std::cout << "情况（2.2）父节点未满，直接插入；" << std::endl;
        pFatherNode->internalNodeInsert(key, pRightSonNode);
//        rootNode = pFatherNode;
        flush({pFatherNode, pRightSonNode});
        deleteNodes({pFatherNode, pRightSonNode});
        return true;
    }
        // 情况（2.3）父节点已满，分裂父节点
    else {
        std::cout << "情况（2.3）父节点已满，分裂父节点" << std::endl;
        Node *pNewInternalNode = createNode(INTERNAL_NODE);
        Node *pOldInternalNode = pFatherNode; // 命名统一
        // 分裂父节点节点进行分裂，upperKey是新节点的 keys[0]
        Key_t upperKey = pOldInternalNode->internalNodeSplit(pNewInternalNode, key);
        if (key > upperKey) {
            // 将子节点插入到新父节点
            pNewInternalNode->internalNodeInsert(key, pRightSonNode);
        } else if (key < upperKey) {
            // 将子节点插入到原父节点
            pOldInternalNode->internalNodeInsert(key, pRightSonNode);
        } else {
            // 将子节点掺入到新父节点的 children[0]，即最左侧。
            pNewInternalNode->children[0] = pRightSonNode->self;
            pRightSonNode->father = pFatherNode->self;
        }

        // 由于对父节点进行了分裂，因此需要将分裂的新节点插入到祖父节点中

        off_t pGrandFatherNodeOffset = pFatherNode->father;

        // 祖父节点不存在，即父节点就是根节点，此时新建父节点
        if (pGrandFatherNodeOffset == INVALID) {
            Node *pNewRootNode = createNode(INTERNAL_NODE);
            pNewRootNode->keys[0] = upperKey;
            pNewRootNode->children[0] = pOldInternalNode->self;
            pNewRootNode->children[1] = pNewInternalNode->self;
            pOldInternalNode->father = pNewRootNode->self;
            pNewInternalNode->father = pNewRootNode->self;
            pNewRootNode->keyNums = 1;
            root = pNewRootNode->self;
            rootNode = pNewRootNode;
            flush({pOldInternalNode, pNewInternalNode, pNewRootNode, pRightSonNode});
            deleteNodes({pOldInternalNode, pNewInternalNode, pNewRootNode, pRightSonNode});
            return true;
        } else {
            // 祖父节点存在，调用该递归函数，将分裂的新内部节点插入祖父节点中。
            Node *pGrandFatherNode = Node::deSerialize(pGrandFatherNodeOffset);
            flush({pOldInternalNode, pNewInternalNode, pRightSonNode});
            return InsertInternalNode(pGrandFatherNode, upperKey, pNewInternalNode);
        }

    }
}

/**
 * 基于索引定位到理论应该插入的叶子节点
 * @param key 索引
 * @return 理论应该插入的叶子节点
 */
Node *BPT::locateLeafNode(Key_t key) {
    cout << "BPT::locateLeafNode：开始定位" << endl;
    int i;
    Node *pNode = this->rootNode;

    cout << "BPT::locateLeafNode：root："<< this->root << endl;
    cout << "BPT::locateLeafNode：pNode.self："<< pNode->self  << endl;
    cout << "BPT::locateLeafNode：keys[0]："<< this->rootNode->keys[0]  << endl;

    while (pNode != NULL) {
        cout << "BPT::locateLeafNode：进入循环"<< endl;
        // 如果是叶子节点，则终止循环
        if (pNode->type == LEAF_NODE) {
            cout << "BPT::locateLeafNode：定位叶子节点成功" << endl;
            break;
        }
        cout << "BPT::locateLeafNode：kekyNums: "<<pNode->keyNums<< endl;
        for (i = 0; i < pNode->keyNums; i++) {
            // 找到第一个键值大于key的位置，没找到，则是最右边那个子节点。
            if (key < pNode->keys[i]) {
                break;
            }
        }
        cout << "BPT::locateLeafNode：i: "<< i<< endl;

        off64_t childrenOffset = pNode->children[i];
        cout << "BPT::locateLeafNode：childrenOffset: "<< childrenOffset<< endl;
        deleteNodes({pNode});
        pNode = Node::deSerialize(childrenOffset);
    }
    return pNode;

}

/**
 * 打印当前节点及其子节点，传入 root 即可打印整棵树。
 * @param pNode
 */
void BPT::printTree(Node *pNode) {
//    std::cout << "BPT::printTree: 根节点偏移量：" << this->root << std::endl;
//    std::cout << "BPT::printTree: 当前节点总数：" << this->nodeNums << std::endl;
//    std::cout << "BPT::printTree: 新节点偏移量：" << this->nextNew << std::endl;
    if (pNode == NULL) {
        return;
    }
    pNode->printNode();
    for (int i = 0; i < pNode->keyNums + 1; i++) {
        Node *children = Node::deSerialize(pNode->children[i]);
        printTree(children);
    }
}

/**
 * 创建一个节点，并修改 bpt 的元数据
 * @param type
 * @return
 */
Node *BPT::createNode(Type_t type) {
    Node *newNode = new Node(type, this->nextNew);
    cout << "BPT::createNode： 当前创建的新节点的偏移量： " << newNode->self << endl;
    int64_t spaceSize = newNode->getNodeSpaceSize();
    cout << "BPT::createNode： 当前创建的新节点的空间大小： " << spaceSize << endl;
    this->nodeNums += 1;
    this->nextNew += spaceSize;
    cout << "BPT::createNode： 更新节点总数为： " << this->nodeNums << endl;
    return newNode;
}


void BPT::serialize() {
    int fd = open("../my.ibd", O_RDWR | O_CREAT, 0664);
    if (-1 == fd) {
        perror("BPT::serialize open");
        printf("errno = %d\n", errno);
    }

    if (-1 == lseek(fd, 0, SEEK_SET)) {
        perror("lseek");
    }
    auto *writeBuffer = (int64_t *) malloc(4096); //  todo #define metasize 4096?
    int64_t p = 0;
    writeBuffer[p++] = this->root;
    writeBuffer[p++] = this->nextNew;
    writeBuffer[p++] = this->nodeNums;
    write(fd, writeBuffer, 4096);
    if (-1 == close(fd)) {
        perror("close");
    }
    free(writeBuffer);
}

BPT *BPT::deSerialize() {
    //  检测文件是否存在
    if (-1 == open("../my.ibd", O_RDWR | O_CREAT | O_EXCL, 0664)) {
        // 文件存在
        if (17 == errno) {
            cout << "BPT::DESerialize  文件存在" << std::endl;
            int fd = open("../my.ibd", O_RDWR | O_CREAT, 0664);
            if (-1 == fd) {
                perror("BPT::deSerialize open");
                printf("errno = %d\n", errno);
            }
            if (-1 == lseek(fd, 0, SEEK_SET)) {
                perror("lseek");
            }
            auto *readBuffer = (int64_t *) malloc(4096); //  todo #define metasize 4096?
            read(fd, readBuffer, 4096);
            BPT *bpt = new BPT();
            cout << readBuffer[0] << endl;
            int64_t p = 0;
            bpt->root = readBuffer[p++];
            bpt->nextNew = readBuffer[p++];
            bpt->nodeNums = readBuffer[p++];
            bpt->rootNode = Node::deSerialize(bpt->root);
            cout << "rootNode 在这" << bpt->rootNode->self << endl;
            if (-1 == close(fd)) {
                perror("close");
            }
            free(readBuffer);
            return bpt;
        }
    } else {
        std::cout << "文件不存在，创建根节点" << std::endl;
        // 文件不存在
        // 整棵树是空的，创建第一个节点
        BPT *bpt = new BPT();
        bpt->root = 4096;
        bpt->nodeNums = 0;
        bpt->nextNew = 4096;
        Node *genesisNode = bpt->createNode(LEAF_NODE);
        bpt->rootNode = genesisNode;
        bpt->serialize();
        return bpt;
    }
}

bool BPT::flush(initializer_list<Node *> nodeList) {
    for (Node *node: nodeList) {
        node->serialize();
    }
//        this->rootNode = Node::deSerialize(this->root); // update: 无需重新获取，rootNode 不会被 delete！
    this->serialize();
}

bool BPT::deleteNodes(initializer_list<Node *> nodeList) {
    for (Node *node: nodeList) {
        if (node->self != this->root){
            delete node;
        }
    }
}

