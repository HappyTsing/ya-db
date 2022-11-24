#include "../include/Node.h"
#include "../include/Utils.h"
#include <iostream>
#include <cstring>
#include <sstream>

using namespace std;


Node::Node(NODE_TYPE type) {
    this->type = type;
    // 初始化，TODO 在文件末尾为该NODE分配一个 4096 字节的区域
//    this->self =
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
bool Node::leafNodeInsert(key_t key, Record value) {

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

    // UPDATE: 直接序列化存入
    utils::writeNode(this->self,this);
    return true;
}

/**
 * 当叶子节点的 keyNums = MAX_KEY, 无法存储更多叶子节点，需要对当前的叶子节点进行分裂。
 * 主要工作是：把原叶子结点的后一半的 key、value 复制到新的叶子结点中。
 * 注意：当原叶子节点的 key 和 value 在复制之后，应该置为 INVALID
 * @param pNewLeafNode 叶子节点分裂后变成两个叶子节点，其中左节点为原节点，右节点为新节点。
 * @return 新节点的keys[0]，用于方便后续插入父节点。
 */
key_t Node::leafNodeSplit(Node *pNewLeafNode) {

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
bool Node::internalNodeInsert(key_t key, Node *pNewChildNode) {

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
    children[i + 1] = pNewChildNode;
    pNewChildNode->father = this;
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
key_t Node::internalNodeSplit(Node *pNewInternalNode, key_t key) {

    // 如果节点未满，不允许分裂，返回空指针
    if (keyNums != MAX_KEY) {
        return -1; // TODO: throw exception
    }

    // 情况（1）upperKey = key
    if ((keys[ORDER / 2 - 1] < key) && (key < keys[ORDER / 2])) {
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
            children[i]->father = pNewInternalNode;  // 重新设置子节点的父亲
            children[i] = nullptr;
            j++;
        }
        pNewInternalNode->keyNums = MAX_KEY - ORDER / 2;
        keyNums = ORDER / 2;
        return key;
    }
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
        // 例如 5 阶的树，MAX_KEY = 4，把后 1 个key 移到新节点
        // 例如 6 阶的树，MAX_KEY = 5，把后 1 个key 移到新节点
        position = ORDER / 2;
    }
    key_t upperKey = keys[ORDER / 2];

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
        children[i]->father = pNewInternalNode;
        children[i] = nullptr;
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
 * |type==LEAFNODE    | keyNums | self | father | rightBrother | keys(array) | values(array(vector)) |
 * |type==INTERNALNODE| keyNums | self | father | rightBrother | keys(array) | children(array) |
 */
void Node::serialize(char *buffer) {
    stringstream ss;

    ss << '|' << type << '|' << keyNums << '|' << self << '|' << father << '|' << rightBrother << '|';

    for (int i = 0; i < keyNums; i++) {
        if (i == keyNums - 1) {
            ss << keys[i] << '|';

        } else {
            ss << keys[i] << ',';
        }
    }

    if (type == LEAF_NODE) {
        int valueNums = keyNums;
        for (int i = 0; i < valueNums; i++) {
            for (int j = 0; j < values[i].size(); j++) {
                if (j == values[i].size() - 1) {
                    ss << values[i][j];
                } else {
                    ss << values[i][j] << '#';
                }
            }
            if (i == valueNums - 1) {
                ss << '|';
            } else {
                ss << ',';
            }
        }

    } else if (type == INTERNAL_NODE) {
        int childrenNums = keyNums + 1;
        for (int i = 0; i < childrenNums; i++) {
            if (i == childrenNums - 1) {
                ss << children[i] << '|';

            } else {
                ss << children[i] << ',';
            }
        }
    }
    ss >> buffer;
}


Node *Node::deSerialize(char *buffer) {
    vector<string> nodeDataVector = utils::stringToVector(buffer,'|');
    NODE_TYPE d_type = stoi(nodeDataVector[0]) == 1 ? INTERNAL_NODE : LEAF_NODE;
    int d_keyNums = stoi(nodeDataVector[1]);
    off_t d_self = stoll(nodeDataVector[2]);
    off_t d_father = stoll(nodeDataVector[3]);
    off_t d_rightBrother = stoll(nodeDataVector[4]);

    Node *node = new Node(d_type);
    node->keyNums = d_keyNums;
    node->self = d_self;
    node->father = d_father;
    node->rightBrother =d_rightBrother;

    vector<string> keysDataVector = utils::stringToVector(nodeDataVector[5],',');
    for(int i = 0; i <d_keyNums;i++){
        node->keys[i] = stoi(keysDataVector[i]);
    }

    if(d_type == LEAF_NODE){
        vector<string> valuesDataVector = utils::stringToVector(nodeDataVector[6],',');
        for(int i=0;i<d_keyNums;i++) {
            vector<string> recordDataVector = utils::stringToVector(valuesDataVector[i],'#');
            for(string item:recordDataVector){
                node->values[i].push_back(stoll(item));
            }
        }
    }else{
        vector<string> childrenDataVector = utils::stringToVector(nodeDataVector[6],',');
        int childrenNums = d_keyNums + 1;
        for(int i = 0; i <childrenNums;i++){
            node->keys[i] = stoi(childrenDataVector[i]);
        }
    }
    return node;
}