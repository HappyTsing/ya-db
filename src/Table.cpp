#include "../include/Table.h"

Table::Table(string tableName, int64_t columnNums) {
    this->tableName = tableName;
    this->columnNums = columnNums;
    this->nextNew = INVALID;
    this->hasIndexColumns.assign(columnNums, false);
    this->columnIndexRootOffsets.assign(columnNums, INVALID);
    this->columnNames.assign(columnNums, "");
}

Table::~Table() = default;

/**
 * 创建表，将创建一个同名文件，里面存储索引和数据
 * （1）id 作为主键索引，主键索引为聚集索引，叶子节点存储行数据
 * （2）其余列（不可重复）可以为其创建非聚集索引，叶子节点仅存储行数据的 id 值，需二次进行主键索引获取数据
 * @param tableName 表名
 * @param columnNames 列名
 * @return 表指针
 */
Table *Table::createTable(string tableName, vector<string> columnNames) {
    string tableFilePath = "../tables/" + tableName + ".table";

    if (-1 == open(tableFilePath.c_str(), O_RDWR | O_CREAT | O_EXCL, 0664)) {
        // 表文件已经存在，说明表已经被创建过
        if (17 == errno) {
            string msg = "[Table::createTable ERROR] Table 「" + tableName + "」already exists, create table fail.";
            cout << msg << endl;
            throw msg;
        }
    }

    Table *table = new Table(tableName, columnNames.size());
    table->nextNew = 8192;
    table->columnNames = columnNames;

    // 建立主键索引的 B+ 树的 genesisNode
    if (table->columnNames[0] == "id") { // TODO 默认传入的列中，第一列必须是 id，将其作为主键索引
        // 更新表元数据
        table->hasIndexColumns[0] = true;
        // 表元数据共分配 8192 字节，在文件的最顶侧。因此第一个节点在文件中的起始偏移量为 8192
        table->columnIndexRootOffsets[0] = 8192;
        Node *genesisNode = new Node(LEAF_NODE, table->nextNew, table->columnNums, table->tableName);
        // 更新下一个节点的起始偏移量
        table->nextNew += genesisNode->getNodeSpaceSize();
        // 节点持久化
        genesisNode->serialize();
    } else {
        string msg = "[Table::createTable ERROR] First column' name must be 「id」.";
        cout << msg << endl;
        throw msg;
    }
    // 表元数据持久化
    table->serialize();
    return table;
}

/**
 * 使用指定表
 * @param tableName 表名
 * @return 表指针
 */
Table *Table::useTable(string tableName) {
    Table *table = nullptr;
    string tableFilePath = "../tables/" + tableName + ".table";
    if (-1 == open(tableFilePath.c_str(), O_RDONLY)) {
        // 表文件不存在，说明未创建表格
        string msg = "[Table::useTable ERROR] use Table 「" + tableName + "」 fail, is it created？";
        cout << msg << endl;
        throw msg;
    } else {
        table = Table::deSerialize(tableName);
    }
    cout << "[Table::useTable INFO] use Table 「" + tableName + "」 success." << std::endl;
    return table;
}

/**
 * 创建索引
 * @param columnName 列名，为指定列创建索引
 * @return 是否创建成功
 */
bool Table::createIndex(string columnName) {
    // 获取第几列，以 0 起始
    int64_t columnNo = getColumnNo(columnName);

    // 判断是否已经创建过索引
    if (this->hasIndexColumns[columnNo]) {
        cout << "[Table::createIndex INFO] column 「" + columnName + "」 index already created, create index fail."
             << std::endl;
        return false;
    }

    // 创建索引树的 genesisNode
    this->hasIndexColumns[columnNo] = true;
    this->columnIndexRootOffsets[columnNo] = this->nextNew;
    Node *genesisNode = new Node(LEAF_NODE, this->nextNew, 1, this->tableName);
    this->nextNew += genesisNode->getNodeSpaceSize();
    genesisNode->serialize();
    BPT *indexBPT = createBPT(this->columnIndexRootOffsets[columnNo]);
    BPT *primaryKeyBPT = createBPT(this->columnIndexRootOffsets[0]);
    doCreateIndex(primaryKeyBPT->rootNode, indexBPT, columnNo);

    this->nextNew = indexBPT->nextNew;
    this->columnIndexRootOffsets[columnNo] = indexBPT->root;
    this->serialize();
    cout << "[Table::createIndex INFO] column 「" + columnName + "」 create index success." << std::endl;
    return true;
}

/**
 * 遍历主键索引的所有叶子节点，将指定列的数据作为 key，对应的主键 id 作为 value，不断使用 insert() 创建非主键索引树
 * @param pNode 主键索引树的 rootNode
 * @param indexBPT 新建的非主键索引树
 * @param lineNumber 列号，以 0 起始
 */
void Table::doCreateIndex(Node *pNode, BPT *indexBPT, int lineNumber) {
    if (pNode == NULL) {
        return;
    }

    if (pNode->type == LEAF_NODE) {
        for (int i = 0; i < pNode->keyNums; i++) {
            Record record = {};
            record.push_back(pNode->keys[i]);
            indexBPT->insert(pNode->values[i][lineNumber], record);
        }
    }

    for (int i = 0; i < pNode->keyNums + 1; i++) {
        Node *children = Node::deSerialize(pNode->children[i], this->tableName);
        doCreateIndex(children, indexBPT, lineNumber);
    }
}

/**
 * 插入记录行，同步更新所有索引
 */
bool Table::insertRecord(Record record) {

    // 默认以第一列，id作为索引。
    Key_t id = record[0];
    BPT *primaryKeyBPT = createBPT(this->columnIndexRootOffsets[0]);
    // 记录行写入主键索引的叶节点中
    primaryKeyBPT->insert(id, record);
    this->nextNew = primaryKeyBPT->nextNew;
    this->columnIndexRootOffsets[0] = primaryKeyBPT->root;
    delete primaryKeyBPT;

    // 更新所有的索引树
    for (int i = 1; i < this->columnNums; i++) {
        // 判断该列是否存在索引，存在则更新
        if (this->hasIndexColumns[i]) {
            cout << "[Table::insertRecord INFO] update column 「" + this->columnNames[i] + "」 index." << endl;
            BPT *indexBPT = createBPT(this->columnIndexRootOffsets[i]);
            vector<int64_t> indexRecord(1);
            indexRecord[0] = record[0];
            indexBPT->insert(record[i], indexRecord);
            this->nextNew = indexBPT->nextNew;
            delete indexBPT;
        }
    }
    this->serialize();
    return true;
}


/**
 * 对某一列进行范围搜索
 * （1）主键索引: 建立 id 的 B+ 树，叶子节点即数据
 * （2）非主键索引: 建立索引列的 B+ 树，叶子节点存放 id，通过 id 进行二次索引获取数据
 * （3）无索引: 遍历所有叶子节点，匹配指定列的数据
 * @param start
 * @param end
 * @param columnName
 */
void Table::selectRecord(Key_t start, Key_t end, string columnName) {
    int64_t columnNo = getColumnNo(columnName);
    BPT *primaryKeyBPT = createBPT(this->columnIndexRootOffsets[0]);
    vector<Record> recordList;

    // 存在索引
    if (hasIndexColumns[columnNo]) {
        Key_t startPrimaryKey, endPrimaryKey;
        // 非主键索引
        if (columnNo != 0) {
            cout << "[Table::selectRecord INFO] 非主键索引" << std::endl;
            BPT *indexBPT = createBPT(this->columnIndexRootOffsets[columnNo]);
            vector<Record> primaryKeyList = indexBPT->search(start, end);
            startPrimaryKey = primaryKeyList.front()[0];
            endPrimaryKey = primaryKeyList.back()[0];
            delete indexBPT;
        } else {
            cout << "[Table::selectRecord INFO] 主键索引" << std::endl;
            startPrimaryKey = start;
            endPrimaryKey = end;
        }
        recordList = primaryKeyBPT->search(startPrimaryKey, endPrimaryKey);
    } else {
        cout << "[Table::selectRecord INFO] 无索引" << std::endl;
        // 不存在索引
        recordList = primaryKeyBPT->searchByValue(start, end, columnNo);
    }
    showRecord(recordList);
}

void printSplit(int64_t columnNums) {
    for (int i = 0; i < columnNums; i++) {
        if (i == 0) {
            cout << "+-------------+";
        } else if (i == columnNums - 1) {
            cout << "-------------+" << endl;

        } else {
            cout << "-------------+";
        }
    }
}

void printTableHeader(vector<string> columnNames) {
    std::cout << "| ";
    for (string columnName: columnNames) {
        cout << columnName;
        int spaceNum = 12 - columnName.length();
        for (int j = 0; j < spaceNum; j++) {
            cout << " ";
        }
        cout << "| ";
    }
    std::cout << std::endl;
}

void printRecord(Record record) {
    std::cout << "| ";
    for (int64_t columnValue: record) {
        cout << columnValue;
        int spaceNum = to_string(((int64_t) 100000000000 / columnValue)).length();

        bool flag = true;
        for(int i = 0; i < to_string(columnValue).length(); i++) {
            int j = stoi(to_string(columnValue).substr(i,i+1));
            if(i == 0 && j == 1){
                continue;
            }
            if(i!=0 && j==0){
                continue;
            }
            flag = false;
        }

        if(flag){
            spaceNum -= 1;
        }


        for (int j = 0; j < spaceNum; j++) {
            cout << " ";
        }
        cout << "| ";
    }
    std::cout << std::endl;
}

void Table::showRecord(vector<Record> recordList) {
    if (recordList.empty()) {
        cout << "[Table::showRecord INFO] record not found" << std::endl;
        return;
    }
    printSplit(this->columnNums);
    printTableHeader(this->columnNames);
    printSplit(this->columnNums);
    for (int i = 0; i < recordList.size(); i++) {
        Record record = recordList[i];
        printRecord(record);
    }
    printSplit(this->columnNums);
}

/**
 * 序列化表格元数据，共分配 8192 字节
 * 1042 | columnNums              | nextNew                 |
 * 4096 |    columnName1(id)      |   columnName2           | ... |
 * 1024 |    true                 |   false                 | ... |
 * 2048 |    主键索引 root 偏移量    |   非主键索引偏移 root 量   | ... |
 */
void Table::serialize() {
//    cout <<"[Table::serialize INFO] serialize Table 「" + tableName + "」" << endl;
    string tableFilePath = "../tables/" + tableName + ".table";

    int fd = open(tableFilePath.c_str(), O_WRONLY | O_CREAT, 0664);
    if (-1 == fd) {
        perror("[Table::serialize ERROR] open");
    }

    if (-1 == lseek(fd, 0, SEEK_SET)) {
        perror("[Table::serialize ERROR] lseek");
    }
    auto *metaBuffer = (int64_t *) malloc(1024);
    char *columnNameBuffer; // 4096
    auto *hasIndexColumnBuffer = (bool *) malloc(1024);
    auto *columnIndexRootBuffer = (int64_t *) malloc(2048);

    int64_t p = 0;
    metaBuffer[p++] = this->columnNums;
    metaBuffer[p++] = this->nextNew;
    write(fd, metaBuffer, 1024);

    string allColumnNames;
    for (string columnName: this->columnNames) {
        allColumnNames.append("#").append(columnName);
    }
    columnNameBuffer = (char *) allColumnNames.c_str();
    write(fd, columnNameBuffer, 4096);

    for (p = 0; p < this->columnNums; p++) {
        hasIndexColumnBuffer[p] = this->hasIndexColumns[p];
    }
    write(fd, hasIndexColumnBuffer, 1024);

    for (p = 0; p < this->columnNums; p++) {
        columnIndexRootBuffer[p] = columnIndexRootOffsets[p];
    }
    write(fd, columnIndexRootBuffer, 2048);

    if (-1 == close(fd)) {
        perror("[Table::serialize ERROR] close");
    }
    free(metaBuffer);
    free(hasIndexColumnBuffer);
    free(columnIndexRootBuffer);
}

/**
 * 反序列化
 * @param tableName 表名
 * @return 表指针
 */
Table *Table::deSerialize(string tableName) {
    cout << "[Table::serialize INFO] deSerialize Table 「" + tableName + "」" << endl;
    //  确保文件存在，即已经createTable。检测文件是否存在
    string tableFilePath = "../tables/" + tableName + ".table";

    int fd = open(tableFilePath.c_str(), O_RDONLY);
    if (-1 == fd) {
        perror("[Table::deSerialize ERROR] open");
    }
    if (-1 == lseek(fd, 0, SEEK_SET)) {
        perror("[Table::deSerialize ERROR] lseek");
    }
    auto *metaBuffer = (int64_t *) malloc(1024);
    char *columnNameBuffer = (char *) malloc(4096);
    auto *hasIndexColumnBuffer = (bool *) malloc(1024);
    auto *columnIndexRootBuffer = (int64_t *) malloc(2048);

    int64_t p = 0;
    read(fd, metaBuffer, 1024);
    int64_t columnNums = metaBuffer[p++];
    Table *table = new Table(tableName, columnNums);

    table->nextNew = metaBuffer[p++];

    read(fd, columnNameBuffer, 4096);
    p = 0;
    char splitPattern = '#';
    char *columnName = strtok(columnNameBuffer, &splitPattern);
    while (columnName != nullptr) {
        table->columnNames[p++] = columnName;
        columnName = strtok(NULL, &splitPattern);
    }

    p = 0;
    read(fd, hasIndexColumnBuffer, 1024);
    for (int i = 0; i < table->columnNums; i++) {
        table->hasIndexColumns[p++] = hasIndexColumnBuffer[i];
    }

    p = 0;
    read(fd, columnIndexRootBuffer, 2048);
    for (int i = 0; i < table->columnNums; i++) {
        table->columnIndexRootOffsets[p++] = columnIndexRootBuffer[i];
    }


    if (-1 == close(fd)) {
        perror("[Table::deSerialize ERROR] close");
    }
    free(metaBuffer);
    free(columnNameBuffer);
    free(hasIndexColumnBuffer);
    free(columnIndexRootBuffer);
    return table;

}

int64_t Table::getColumnNo(string columnName) {
    int no;
    // 寻找索引是第几列
    for (no = 0; no < columnNums; no++) {
        if (columnName == columnNames[no]) {
            break;
        }
    }
    if (no == columnNums) {
        string msg = "[Table::getColumnNo ERROR] columnName 「" + columnName + "」not found.";
        cout << msg << endl;
        throw msg;
    }
    return no;
}

/**
 * 创建 B+ 树实例
 * 默认第一列为主键 id，因此如果创建主键索引的 B+ 树，则其叶子节点存储了若干个记录行 vector<int64_t> record(columnNums)
 * 如果创建非主键索引的 B+ 树，则其叶子节点仅存储 id： vector<int64_t> record(1)，即该 vector 中仅有一个元素。
 * @param root B+ 树根节点的文件便宜
 */
BPT *Table::createBPT(off64_t root) {

    BPT *bpt = NULL;
    if (root == this->columnIndexRootOffsets[0]) {
        bpt = new BPT(root, this->nextNew, this->columnNums, this->tableName);
    } else {
        bpt = new BPT(root, this->nextNew, 1, this->tableName);
    }
    return bpt;
}