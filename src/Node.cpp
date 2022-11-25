#include "../include/Node.h"


Node::Node(NODE_TYPE type, off64_t selfOffset) {
    this->type = type;
    this->self = selfOffset;
    this->keyNums = 0;
    this->father = INVALID;
    this->rightBrother = INVALID;
    memset(children, INVALID, MAX_CHILDREN);
    memset(this->keys, INVALID, MAX_KEY);
    memset(this->values, INVALID, MAX_KEY);
}

Node::~Node() = default;

/**
 * 将 key 和 value 插入到未满的叶子节点中，需要插入 key 和 value
 * @param key 索引
 * @param value 数据
 * @return 是否插入成功
 */
bool Node::leafNodeInsert(Key_t key, Record value) {

    // 如果叶子节点已满，直接返回失败
    if (keyNums >= MAX_KEY) {
        return false;
    }
    int i, j;

    // 比较通过比较索引大小，获取数据应该插入的位置
    for (i = 0; i < keyNums; i++) {
        if (key < keys[i]) {
            break;
        }
    }

    // 当前位置以及其后面的 key 和 value 依次后移，空出当前位置
    for (j = keyNums; j > i; j--) {
        keys[j] = keys[j - 1];
        values[j] = values[j - 1];
    }
    // 写入新的 key 和 value
    keys[i] = key;
    values[i] = value;
    keyNums++;
    return true;
}

/**
 * 当叶子节点的 keyNums = MAX_KEY, 无法存储更多叶子节点，需要对当前的叶子节点进行分裂。
 * 主要工作是：把原叶子结点的后一半的 key、value 复制到新的叶子结点中。
 * 注意：当原叶子节点的 key 和 value 在复制之后，应该置为 INVALID
 * @param pNewLeafNode 叶子节点分裂后变成两个叶子节点，其中左节点为原节点，右节点为新节点。
 * @return 新节点的keys[0]，用于方便后续插入父节点。
 */
Key_t Node::leafNodeSplit(Node *pNewLeafNode) {

    // 如果叶子节点未满，不允许分裂，返回空指针
    if (keyNums != MAX_KEY) {
        std::cout << "当叶子节点未满，不允许分裂！" << std::endl;
        return INVALID; //TODO 排除异常
    }

    int j = 0;
    // C++ 除法自动向下取整。例如 5 阶的树，MAX_KEY = 4，此时将后 2 个 key 和 value 移动到新的叶子节点中。
    //                    例如 6 阶的树，MAX_KEY = 5，此时将后 2 个 key 和 value 移动到新的叶子节点中。
    //                    此时新的叶子节点的第一个key，将作为其父节点的key。
    for (int i = ORDER / 2; i < MAX_KEY; i++) {
        // 迁移后半部分 key、value 至新节点
        pNewLeafNode->keys[j] = keys[i];
        pNewLeafNode->values[j] = values[i];

        // 原节点已迁移的 key、value 置为无效 INVALID
        keys[i] = INVALID;
//        values[i] = INVALID;
        j++;
    }

    // 修改两个叶子节点的 keyNums 值
    pNewLeafNode->keyNums = j;
    this->keyNums -= j;
    return pNewLeafNode->keys[0];
}

/**
 * 当节点（LeafNode/internalNode）分裂时，必须将新节点的keys[0]插入其父节点中，且父节点必须指向该新节点，新节点的 father 指针指向其父节点。
 * @param key pNewChildNode->keys[0]，即最小的key值
 * @param pNewChildNode 分裂后的新节点
 * @return 是否插入成功
 */
bool Node::internalNodeInsert(Key_t key, Node *pNewChildNode) {

    // 如果中间节点已满，直接返回失败
    if (keyNums >= MAX_KEY) {
        return false;
    }
    int i, j;
    // 通过比较索引大小，获取插入位置
    for (i = 0; i < keyNums; i++) {
        if (key < keys[i]) {
            break;
        }
    }

    // 当前位置以及其后面的 key 依次后移，空出插入位置
    for (j = keyNums; j > i; j--) {
        keys[j] = keys[j - 1];
    }

    // 默认 孩子节点数 = keyNums + 1
    // 指针后移，控住插入位置
    // 值得注意的是，插入位置永远不会是 children[0]，因此想要插入到 children[0] 时，应该直接显式调用：children[0] = xxx，而不是调用该方法。
    for (j = keyNums + 1; j > i + 1; j--) {
        children[j] = children[j - 1];
    }

    // 写入新的 key 和 children 指针
    keys[i] = key;
    children[i + 1] = pNewChildNode->self;
    pNewChildNode->father = this->self;
    keyNums++;
    return true;
}

/**
 * 当子节点分裂时，需要将分裂后新的子节点插入其父节点中，如果父节点已满，则需要对父节点进行分裂。
 * 分裂中间节点与叶子结点并不相同，无法单纯的一分为二，因为其不仅有 keyNums 个 key，还有 keyNums + 1 个孩子节点指针。
 * 假设 5 阶的树，MAX_KEY = 4，如果直接一分为二，则每个内部节点有 2 个 key，此时共有 3*2 个孩子节点指针。
 * 5 阶的树 MAX_CHILDREN = 5，显然直接一分为二是错误的。
 * 因此，应该让其中一个内部节点有 1 个 key，另一个内部节点 2 个 key，此时正好 2 + 3 = 5 个孩子节点指针。
 * 而少了的那个key，应该向上插入到其父节点上，此处将其命名为 upperKey。
 * 由于 b+ tree 是有序的，因此 upperKey 的大小应该是位于中间。
 * 根据插入的 key 的不同，存在三种情况
 * 假设阶数为 5，此时内部节点存了：1 3 5 7
 * (1) 当插入的 key = 4 时，此时内部节点分为两个 【1 3】 和 【5 7】，upperKey = key = 4。
 *     分裂后，将 key = 4 的新节点，作为分裂后，右侧内部节点的第一个子节点☆。
 * (2) 当插入的 key < 3，例如 2 时，此时内部节点分为两个 【1】 和 [5 7] ，upperKey = 3。
 *     分裂后，将 key = 3 的新节点，插入左侧内部节点
 * (3) 当插入的 key > 5，例如 9 时，此时内部节点分为两个 【1 3】和 【7】，upperKey = 5。
 *     分裂后，将key = 4的新节点，插入右侧内部节点
 * @return
 */
Key_t Node::internalNodeSplit(Node *pNewInternalNode, Key_t key) {

    // 如果节点未满，不允许分裂，返回空指针
    if (keyNums != MAX_KEY) {
        return -1; // TODO: throw exception
    }

    // 情况（1）upperKey = key
    if ((keys[ORDER / 2 - 1] < key) && (key < keys[ORDER / 2])) {
        std::cout << "情况（2.2）(1)upperKey = key" << std::endl;
        // key 移动到新节点
        // 例如 5 阶的树，MAX_KEY = 4，把后 2 个key 移到新节点
        // 例如 6 阶的树，MAX_KEY = 5，把后 2 个key，移到新节点
        int j = 0;
        for (int i = ORDER / 2; i < MAX_KEY; i++) {
            pNewInternalNode->keys[j] = keys[i];
            keys[i] = INVALID;
            j++;
        }

        // 指针 移到新节点
        // 例如 5 阶的树，MAX_CHILDREN = 5，把后 2 个 指针 移到新节点
        // 例如 6 阶的树，MAX_CHILDREN = 6，把后 2 个 指针 移到新节点
        // children[0]并未指向任意节点，后续将指向插入该中间节点的子节点。
        j = 1;
        for (int i = ORDER / 2 + 1; i < MAX_CHILDREN; i++) {
            pNewInternalNode->children[j] = children[i];
            Node *pchildren = Node::deSerialize(children[i]);
            pchildren->father = pNewInternalNode->self;
            children[i] = INVALID;
//            children[i]->father = pNewInternalNode;  // 重新设置子节点的父亲
//            children[i] = nullptr;
            pchildren->serialize(); //todo 在里面刷新？
            delete pchildren;
            j++;
        }
        pNewInternalNode->keyNums = MAX_KEY - ORDER / 2;
        keyNums = ORDER / 2;
        return key;
    }
    std::cout << "情况（2.2）(2)(3)upperKey != key" << std::endl;
    // 处理情况（2）（3），通过 position 可以巧妙的一起处理，而不用分开处理。
    // position 是数组下标，从0开始。
    int position;
    if (key < keys[ORDER / 2 - 1]) {
        // key在左节点
        // 例如 5 阶的树，MAX_KEY = 4，把后 2 个key 移到新节点
        // 例如 6 阶的树，MAX_KEY = 5，把后 2 个key 移到新节点
        position = ORDER / 2 - 1;
    } else { // key > keys[ORDER/2]
        // key 在右节点
        // 例如 5 阶的树，MAX_KEY = 4，把后 1 个key 移到新节点, position = 2
        // 例如 6 阶的树，MAX_KEY = 5，把后 1 个key 移到新节点
        position = ORDER / 2;
    }
    Key_t upperKey = keys[ORDER / 2];

    int j = 0;
    for (int i = position + 1; i < MAX_KEY; i++) {
        pNewInternalNode->keys[j] = keys[i];
        keys[i] = INVALID;
        j++;
    }

    // 注意，upperKey指向的孩子，将被新的中间节点指向！
    j = 0;
    for (int i = position + 1; i < MAX_CHILDREN; i++) {
        pNewInternalNode->children[j] = children[i];


        Node *pchildren = Node::deSerialize(children[i]);
        pchildren->father = pNewInternalNode->self;
//            children[i]->father = pNewInternalNode;  // 重新设置子节点的父亲
        children[i] = INVALID;
//        children[i]->father = pNewInternalNode;
//        children[i] = nullptr;
        pchildren->serialize(); //todo 在里面刷新？
        delete pchildren;
        j++;
    }
    pNewInternalNode->keyNums = MAX_KEY - position - 1;
    keyNums = position;
    return upperKey;
}

/**
 * 输出节点信息
 */
void Node::printNode() {
    if (type == LEAF_NODE) {
        std::cout << "leaf node  ";
        std::cout << "[ ";
        for (int i = 0; i < keyNums; i++) {
            std::cout << "(" << "index: " << keys[i];
            std::cout << " value:";
            for (long long j: values[i]) {
                std::cout << " " << j;
            }
            std::cout << ")";

        }
        std::cout << "]" << std::endl;
    } else {
        std::cout << "internal node  ";
        std::cout << "[ ";
        for (int i = 0; i < keyNums; i++) {
            std::cout << keys[i] << " ";
        }
        std::cout << "]" << std::endl;
    }
}

/**
 * |type==LEAFNODE    | keyNums | self | father  | keys(array) | hasGrandChildren | values(array(vector)) |
 *
 *
 * |type==INTERNALNODE| keyNums | self | father | keys(array)  | rightBrother  | children(array) |
 */
void Node::serialize() {
    int fd = open("../my.ibd", O_RDWR | O_CREAT, 0664);
    if (-1 == fd) {
        perror("open");
        printf("NODE errno = %d\n", errno);
    }
    if (-1 == lseek(fd, this->self, SEEK_SET)) {
        perror("lseek");
    }
    write(fd, &this->type, sizeof(NODE_TYPE));
//    cout << "type " << sizeof(NODE_TYPE) << endl;

    write(fd, &this->self, sizeof(off64_t));
//    cout << "self " << sizeof(off64_t) << endl;

    write(fd, &this->keyNums, sizeof(int64_t));
//    cout << "keyNums " << sizeof(int64_t) << endl;

    write(fd, &this->father, sizeof(off64_t));
//    cout << "father " << sizeof(off64_t) << endl;

    for (int i = 0; i < this->keyNums; i++) {
        write(fd, &this->keys[i], sizeof(Key_t));
//        cout << "keys " << sizeof(Key_t) << endl;
    }

    int64_t unUsedNodeSpaceSize;
    if (this->type == INTERNAL_NODE) {
        for (int i = 0; i < this->keyNums + 1; i++) {
            write(fd, &this->children[i], sizeof(off64_t));
//            cout << "children " << sizeof(off64_t) << endl;
        }

        // 留出空余空间
        unUsedNodeSpaceSize =
                (MAX_KEY - this->keyNums) * sizeof(Key_t) + (MAX_CHILDREN - this->keyNums - 1) * sizeof(off64_t);
    } else {
        write(fd, &this->rightBrother, sizeof(off64_t));
//        cout << "rightBrother " << sizeof(off64_t) << endl;

//        int64_t recordSize = this->values[0].size(); // todo 此处先写死
        int64_t recordSize = 4;
        write(fd, &recordSize, sizeof(int64_t));
//        cout << "recordSize " << recordSize << endl;

        for (int i = 0; i < this->keyNums; i++) {
            for (int j = 0; j < this->values[i].size(); j++) {
                write(fd, &this->values[i][j], sizeof(off64_t));
//                cout << sizeof(off64_t) << endl;
            }
        }
        unUsedNodeSpaceSize =
                (MAX_KEY - this->keyNums) * sizeof(Key_t) + (MAX_VALUE - this->keyNums) * recordSize * sizeof(int64_t);
    }
//    cout << "unUsedNodeSpaceSize " << unUsedNodeSpaceSize << endl;
    char *fillBuffer = (char *) malloc(unUsedNodeSpaceSize);
    write(fd, fillBuffer, unUsedNodeSpaceSize);
    free(fillBuffer);

    if (-1 == close(fd)) {
        perror("close");
    }
}


Node *Node::deSerialize(off64_t offset) {
    if(offset == INVALID){
        return NULL;
    }

    int fd = open("../my.ibd", O_RDWR | O_CREAT, 0664);
    if (-1 == fd) {
        perror("open");
        printf("errno = %d\n", errno);
    }

    if (-1 == lseek(fd, offset, SEEK_SET)) {
        perror("lseek");
    }

    NODE_TYPE d_type;
    read(fd, &d_type, sizeof(NODE_TYPE));

    off64_t d_self;
    read(fd, &d_self, sizeof(off64_t));
    Node *node = new Node(d_type, d_self);

    read(fd, &node->keyNums, sizeof(int64_t));
    read(fd, &node->father, sizeof(off64_t));

    for (int i = 0; i < node->keyNums; i++) {
        read(fd, &node->keys[i], sizeof(Key_t));
    }
    if (d_type == INTERNAL_NODE) {
//        cout << "INTERNAL_NODE" << endl;

        for (int i = 0; i < node->keyNums + 1; i++) {
            read(fd, &node->children[i], sizeof(off64_t));
        }
    } else {
//        cout << "LEAF_NODE" << endl;
        read(fd, &node->rightBrother, sizeof(off64_t));
        int64_t recordSize;
        read(fd, &recordSize, sizeof(int64_t));

        for (int i = 0; i < node->keyNums; i++) {
            for (int j = 0; j < recordSize; j++) {
                int64_t lineItem;
                read(fd, &lineItem, sizeof(off64_t));
                node->values[i].push_back(lineItem);
            }
        }
    }
    if (-1 == close(fd)) {
        perror("close");
    }
    return node;
}

int64_t Node::getNodeSpaceSize() {
    if (this->type == INTERNAL_NODE) {
        return sizeof(NODE_TYPE) + sizeof(int64_t) + sizeof(off64_t) + sizeof(off64_t) + (MAX_KEY) * sizeof(Key_t)
               + (MAX_CHILDREN) * sizeof(off64_t);
    } else {
//        int64_t recordSize = this->values[0].size(); TODO:  此处先写死，后续是从table中传来
        int64_t recordSize = 4;
        return sizeof(NODE_TYPE) + sizeof(int64_t) + sizeof(off64_t) + sizeof(off64_t) + (MAX_KEY) * sizeof(Key_t)
               + sizeof(off64_t) + sizeof(int64_t) + (MAX_VALUE) * recordSize * sizeof(int64_t);
    }
}
