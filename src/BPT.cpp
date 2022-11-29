#include "../include/BPT.h"

BPT::BPT(off64_t root, off64_t nextNew, int64_t columnNums, string tableName) {
    this->root = root;
    this->nextNew = nextNew;
    this->columnNums = columnNums;
    this->tableName = tableName;
    this->rootNode = getNode(root);
}

BPT::~BPT() = default;

/**
 * 插入数据首先要找到理论上要插入的叶子结点，然后分两种情况：
 * (1) 叶子结点未满，直接在该结点中插入即可；
 * (2) 叶子结点已满，首先分裂叶子节点，然后选择将数据插入原结点或新结点；
 *     分裂后，需要将新节点的 keys[0] 插入父节点中，又分为三种情况：
 *     （2.1）无父节点，即根节点是叶子节点：生成新根节点，插入；
 *     （2.2）父节点未满：修改父节点指针，插入；
 *     （2.3）父节点已满：分裂父节点，选择插入原父节点或新父节点；
 *           由于父节点分裂后，需要将新父节点的 keys[0] 插入祖父节点中，因为祖父节点也可能满，因此需要不断递归直到未满的祖先节点，或者直到满根节点，分裂该根节点，再生成新根节点。
 * @param key 索引
 * @param value 数据，在本项目中指记录行
 * @return 是否插入成功
 */
bool BPT::insert(Key_t key, Record value) {

    // 1. 调用 search()，检查是否重复插入，若重复则直接返回 false
    Record repeatRecord = search(key);
    if (!repeatRecord.empty()) {
        cout << "[BPT::insert ERROR] repeat insert" << endl;
        return false;
    }

    // 2. 定位理论上应该插入的叶子节点
    Node *pLeafNode = locateLeafNode(key);

    // 情况（1）叶子节点未满，直接插入
    if (pLeafNode->keyNums < MAX_KEY) {
        std::cout << "[BPT::insert INFO] 情况（1）叶子节点未满，直接插入" << std::endl;
        pLeafNode->leafNodeInsert(key, value);

        // 持久化并刷新 rootNode 缓存
        flush({pLeafNode});
        // 释放 Node 对象，避免内存泄露
        deleteNodes({pLeafNode});
        return true;
    }
        // 情况（2）叶子节点已满，首先将叶子节点分裂，然后选择将数据插入原结点或新结点；
    else {
        std::cout << "[BPT::insert INFO] 情况（2）叶子节点已满，分裂叶子节点" << std::endl;

        // 分裂叶子节点
        Node *pNewLeafNode = createNode(LEAF_NODE);
        Key_t splitKey = pLeafNode->leafNodeSplit(pNewLeafNode);
        Node *pOldLeafNode = pLeafNode;    // 命名统一

        // 叶子节点相连，通过修改其 rightBrother 为对应节点的 self 偏移量
        off_t OldLeafNodeRightBrotherOffset = pOldLeafNode->rightBrother;
        pOldLeafNode->rightBrother = pNewLeafNode->self;
        pNewLeafNode->rightBrother = OldLeafNodeRightBrotherOffset;

        // 选择数据插入原节点或新节点：如果插入的key，小于新的叶子节点的第一个 key，则说明其应该插入到旧的叶子节点
        if (key < splitKey) {
            pOldLeafNode->leafNodeInsert(key, value); // 插入原节点
        } else {
            pNewLeafNode->leafNodeInsert(key, value); // 插入新节点
        }
        off_t FatherInternalNodeOffset = pLeafNode->father;

        // 情况（2.1）原节点是根节点，生成新根节点
        if (FatherInternalNodeOffset == INVALID) {
            std::cout << "[BPT::insert INFO] 情况（2.1）原节点是根节点，生成新根节点" << std::endl;
            Node *pNewRootNode = createNode(INTERNAL_NODE);
            pNewRootNode->keys[0] = splitKey;
            pNewRootNode->children[0] = pOldLeafNode->self;
            pNewRootNode->children[1] = pNewLeafNode->self;
            pOldLeafNode->father = pNewRootNode->self;
            pNewLeafNode->father = pNewRootNode->self;
            pNewRootNode->keyNums = 1;
            root = pNewRootNode->self;  // 根节点改变，修改当前 B+ 树的根节点偏移量
//            rootNode = pNewRootNode;  // 修改 rootNode，该步骤在 flush() 中统一进行
            flush({pOldLeafNode, pNewLeafNode, pNewRootNode});
            deleteNodes({pOldLeafNode, pNewLeafNode, pNewRootNode});
            return true;
        } else {
            // 从文件中反序列化出叶子节点的父节点
            Node *pFatherInternalNode = getNode(FatherInternalNodeOffset);
            // 原节点更改完毕，持久化存储
            flush({pOldLeafNode});
            deleteNodes({pOldLeafNode});
            std::cout << "[BPT::insert INFO] 情况（2.2）（2.3）抽象出递归函数处理" << std::endl;
            // 情况（2.1）（2.2）抽象出递归函数处理
            // 由于 pFatherInternalNode、pNewLeafNode 后续还要使用，因此不对其进行持久化和内存释放
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

    // 情况（2.2）父节点未满，直接插入
    if (pFatherNode->keyNums < MAX_KEY) {
        std::cout << "[BPT::InsertInternalNode INFO] 情况（2.2）父节点未满，直接插入" << std::endl;
        pFatherNode->internalNodeInsert(key, pRightSonNode);

        // 当父节点为根节点时，更新 rootNode。为什么不会自动更新？因为 pFatherNode 是从文件中反序列化而来，二者并不是同一个对象。
        // 该操作已在 flush() 中统一执行
//        if(this->root == pFatherNode->self){
//            rootNode = pFatherNode;
//        }
        flush({pFatherNode, pRightSonNode});
        deleteNodes({pFatherNode, pRightSonNode});
        return true;
    }
        // 情况（2.3）父节点已满，分裂父节点
    else {
        std::cout << "[BPT::InsertInternalNode INFO] 情况（2.3）父节点已满，分裂父节点" << std::endl;
        Node *pNewInternalNode = createNode(INTERNAL_NODE);
        Node *pOldInternalNode = pFatherNode; // 命名统一
        // 分裂父节点节点进行分裂，upperKey 是新节点的 keys[0]
        Key_t upperKey = pOldInternalNode->internalNodeSplit(pNewInternalNode, key);
        if (key > upperKey) {
            pNewInternalNode->internalNodeInsert(key, pRightSonNode); // 将子节点插入到新父节点
        } else if (key < upperKey) {
            pOldInternalNode->internalNodeInsert(key, pRightSonNode); // 将子节点插入到原父节点
        } else {
            // 此时 key == upperKey，将子节点掺入到新父节点的 children[0]，即最左侧。
            pNewInternalNode->children[0] = pRightSonNode->self;
            pRightSonNode->father = pFatherNode->self;
        }

        // 由于对父节点进行了分裂，因此需要将分裂的新节点插入到祖父节点中
        off_t pGrandFatherNodeOffset = pFatherNode->father;

        // 祖父节点不存在，即父节点就是根节点，此时新建父节点
        if (pGrandFatherNodeOffset == INVALID) {
            std::cout << "[BPT::InsertInternalNode INFO] 情况（2.3.1）祖父节点不存在，生成新根节点" << std::endl;
            Node *pNewRootNode = createNode(INTERNAL_NODE);
            pNewRootNode->keys[0] = upperKey;
            pNewRootNode->children[0] = pOldInternalNode->self;
            pNewRootNode->children[1] = pNewInternalNode->self;
            pOldInternalNode->father = pNewRootNode->self;
            pNewInternalNode->father = pNewRootNode->self;
            pNewRootNode->keyNums = 1;
            root = pNewRootNode->self;
//            rootNode = pNewRootNode;
            flush({pOldInternalNode, pNewInternalNode, pNewRootNode, pRightSonNode});
            deleteNodes({pOldInternalNode, pNewInternalNode, pNewRootNode, pRightSonNode});
            return true;
        } else {
            // 祖父节点存在，调用该递归函数，将分裂的新内部节点插入祖父节点中。
            std::cout << "[BPT::InsertInternalNode INFO] 情况（2.3.2）祖父节点存在，继续递归处理" << std::endl;
            Node *pGrandFatherNode = getNode(pGrandFatherNodeOffset);
            flush({pOldInternalNode, pNewInternalNode, pRightSonNode});
            return InsertInternalNode(pGrandFatherNode, upperKey, pNewInternalNode);
        }
    }
}

/**
 * 基于索引定位到理论应该插入的叶子节点
 * @param key 索引
 * @return 理论应该插入的叶子节点，未查询到时返回 NULL
 */
Node *BPT::locateLeafNode(Key_t key) {
    int i;
    Node *pNode = this->rootNode;

//    十分有用的日志，出错时可查看是否定位问题，经常出错！
//    cout << "[BPT::locateLeafNode INFO] root：" << this->root << endl;
//    cout << "[BPT::locateLeafNode] pNode.self：" << pNode->self << endl;

    while (pNode != NULL) {
        // 如果是叶子节点，则终止循环
        if (pNode->type == LEAF_NODE) {
            cout << "[BPT::locateLeafNode INFO] 定位叶子节点成功" << endl;
            break;
        }
        for (i = 0; i < pNode->keyNums; i++) {
            // 找到第一个键值大于key的位置，没找到，则是最右边那个子节点。
            if (key < pNode->keys[i]) {
                break;
            }
        }
        off64_t childrenOffset = pNode->children[i];
//        cout << "[BPT::locateLeafNode INFO] childrenOffset: " << childrenOffset << endl;
        deleteNodes({pNode});
        pNode = getNode(childrenOffset);
    }
    return pNode;

}

/**
 * 遍历查询，从最左侧的叶子结点（最小值），不断往右的兄弟节点遍历。左右均取闭区间。
 * @param columnValueStart 起始值
 * @param columnValueEnd 终止值
 * @param columnNo 查询第几列
 * @return 查询的行记录，未查询到时返回空 vector
 * @Note 该方法应该仅 主键索引（聚集索引）的 B+ 树进行调用
 */
vector<Record> BPT::searchByValue(int64_t columnValueStart, int64_t columnValueEnd, int64_t columnNo) {
    Node *pNode = this->rootNode;
    vector<Record> recordList = {};
    int64_t i;

    // 找到最左侧的节点
    while (pNode != NULL) {
        if (pNode->type == LEAF_NODE) {
            break;
        }
        // 最左边
        off64_t childrenOffset = pNode->children[0];
        deleteNodes({pNode});
        pNode = getNode(childrenOffset);
    }

    if (pNode == NULL || pNode->keyNums == 0) {
        return recordList;
    }

    // 从左到右遍历所有叶子节点
    while (pNode != nullptr) {
        for (i = 0; i < pNode->keyNums; i++) {
            if ((columnValueStart <= pNode->values[i][columnNo]) && (pNode->values[i][columnNo] <= columnValueEnd)) {
                recordList.push_back(pNode->values[i]);
            }
        }
        // 找到所有数据，停止遍历
        if(recordList.size() == (columnValueEnd - columnValueStart + 1)) {
            break;
        }
        off64_t rightBrotherOffset = pNode->rightBrother;
        deleteNodes({pNode});
        pNode = getNode(rightBrotherOffset);
    }
    return recordList;
}

/**
 * 通过 B+ 树索引查询指定的数据，左右均取闭区间。
 * @param key 索引
 * @return 记录行
 */
Record BPT::search(Key_t key) {
    vector<Record> recordList = search(key, key);
    Record record = {};
    if (recordList.size() == 1) {
        record = recordList[0];
    }
    return record;
}

/**
 * 通过 B+ 树索引查询指定区间的数据。
 * （1）对于主键索引（聚集索引），叶子节点数据中存储了记录行 Record
 * （2）对于非主键索引，叶子节点数据中仅存储了记录行的主键 id，需要再次调用聚集索引的 B+ 树，二次索引才能获取数据
 * @param start 起始索引
 * @param end 终止索引
 * @return 查询的行记录，未查询到时返回空 vector
 */
vector<Record> BPT::search(Key_t start, Key_t end) {
    vector<Record> recordList = {};
    int64_t i;
    Node *pNode = this->rootNode;

    // 从根节点开始，通过比较索引大小，定位到存有目标数据的叶子节点
    while (pNode != NULL) {
        if (pNode->type == LEAF_NODE) {
            break;
        }
        for (i = 0; i < pNode->keyNums; i++) {
            if (start <= pNode->keys[i]) {
                break;
            }
        }
        off64_t childrenOffset = pNode->children[i];
        deleteNodes({pNode});
        pNode = getNode(childrenOffset);
    }

    // 仅根节点 = 叶节点时，pNode->keyNums = 0
    if (pNode == NULL || pNode->keyNums == 0) {
        cout << "[BPT::search INFO] Record Not Found" << endl;
        return recordList;
    }

    //  查找当前叶子节点中 key 大于等于 start 的值
    for (i = 0; i < pNode->keyNums; i++) {
        if ((start <= pNode->keys[i]) && (pNode->keys[i] <= end)) {
            recordList.push_back(pNode->values[i]);
        }
    }

    // 当前节点的最后一个 key 小于 end，则需要继续查询其右兄弟节点
    while (pNode->keys[MAX_KEY - 1] < end) {
        off64_t rightBrotherOffset = pNode->rightBrother;

        // 已经到最右侧
        if (rightBrotherOffset == INVALID) {
            break;
        }
        deleteNodes({pNode});
        pNode = getNode(rightBrotherOffset);
        for (i = 0; i < pNode->keyNums; i++) {
            if ((start <= pNode->keys[i]) && (pNode->keys[i] <= end)) {
                recordList.push_back(pNode->values[i]);
            }
        }
    }
    return recordList;
}


/**
 * 打印当前节点及其子节点
 * @param pNode
 */
void BPT::printTree() {
    doPrintTree(this->rootNode);
}

void BPT::doPrintTree(Node *pNode) {
//    std::cout << "[BPT::printTree INFO]: 根节点偏移量：" << this->root << std::endl;
//    std::cout << "[BPT::printTree INFO]: 当前节点总数：" << this->nodeNums << std::endl;
//    std::cout << "[BPT::printTree INFO]: 新节点偏移量：" << this->nextNew << std::endl;
    if (pNode == NULL) {
        return;
    }
    pNode->printNode();
    for (int i = 0; i < pNode->keyNums + 1; i++) {
        Node *children = getNode(pNode->children[i]);
        doPrintTree(children);
    }
}

/**
 * 创建一个节点，并修改 bpt 的元数据
 * @param type 创建的节点类型
 * @return 节点指针
 */
Node *BPT::createNode(Type_t type) {
    Node *newNode;
    // 叶子节点，需要传入 columnNums，即一条记录有多少列。columnNums 用于计算节点大小。
    if (type == LEAF_NODE) {
        newNode = new Node(type, this->nextNew, this->columnNums, this->tableName);
    } else {
        // 非叶子结点，无需传入columnNums，因为不存储数据记录行
        newNode = new Node(type, this->nextNew, this->tableName);
    }
    int64_t spaceSize = newNode->getNodeSpaceSize();
//    cout << "[BPT::createNode INFO]： 当前创建的新节点的起始偏移量： " << newNode->self << endl;
//    cout << "【BPT::createNode INFO]： 当前创建的新节点的空间大小: " << spaceSize << " bytes" << endl;
    // 更新 nextNew，其为下一个节点在文件中的起始偏移量
    this->nextNew += spaceSize;
    return newNode;
}

/**
 * 从文件中反序列化节点
 * @param self 节点的文件起始偏移量
 * @return 节点指针
 */
Node *BPT::getNode(off64_t self) {
    Node *node = Node::deSerialize(self, this->tableName);
    return node;
}

/**
 * 持久化存储发生改动的节点，如果为根节点，则更新内存中的 rootNode。
 * @param nodeList
 */
void BPT::flush(initializer_list<Node *> nodeList) {
    bool flushRootNode = false;
    for (Node *node: nodeList) {
        if (node->self == this->root) {
            flushRootNode = true;
        }
        node->serialize();
    }
    if (flushRootNode) {
        this->rootNode = getNode(this->root); // update: 无需重新获取，rootNode 不会被 delete！
    }
}

/**
 * 释放不再使用的节点，防止内存泄露。该函数不会释放 rootNode，因为其在整棵树执行过程中始终有用。
 * @param nodeList 需要释放的节点集合
 * @return
 */
void BPT::deleteNodes(initializer_list<Node *> nodeList) {
    for (Node *node: nodeList) {
        if (node->self != this->root) {
            delete node;
        }
    }
}



