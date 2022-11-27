#include <iostream>
#include "../include/Table.h"
#include "../include/BPT.h"



// 初始化的时候新建一个文件夹。 TODO 抽象出一个CreateTable()的方法
Table::Table(string tableName){
    this->tableName = tableName;
    this->columnNums = INVALID;
    this->nextNew = INVALID;
    this->bpt = NULL;
}

//bool Table::createIndex(string columnName) {
//    int i;
//    for(i=0;i<columnNames.size();i++) {
//        if(columnName == columnNames[i]){
//            break;
//        }
//    }
//    off64_t indexOffset = this->columnIndexRootOffsets[i];
//    this->bpt  = new BPT(indexOffset);
//
//    this->bpt->root;
//
//    return false;
//}
//
//void Table::doCreateIndex(Node *pNode,BPT* bpt,int lineNumber) {
//    if (pNode == NULL) {
//        return;
//    }
//    if(pNode->type == LEAF_NODE){
//        for(int i= 0;i<pNode->keyNums; i++){
//            Record record = {};
//            record.push_back(pNode->keys[i]);
//            bpt->insert(pNode->values[i][lineNumber],record);
//        }
//    }
//    for (int i = 0; i < pNode->keyNums + 1; i++) {
//        Node *children = Node::deSerialize(pNode->children[i]);
//        doCreateIndex(children,bpt,lineNumber);
//    }
//}

bool Table::insertRecord(Record record){
    // 默认以第一列，id作为索引。
    off64_t idRoot = this->columnIndexRootOffsets[0]; // 获取下标
    Key_t id = record[0];
    this->bpt = createBPT(idRoot);
    bpt->insert(id,record);
    this->nextNew = bpt->nextNew;
    this->columnIndexRootOffsets[0] = bpt->root;
    this->serialize();
    return true;
}

BPT* Table::createBPT(off64_t root){
    return new BPT(root,this->nextNew,this->columnNums,this->tableName);
}

Table* Table::createTable(string tableName, initializer_list<char> columnNames){
    Table* table = new Table(tableName);
    string tableFilePath = "../" + tableName;

//    if (-1 == open(tableFilePath.c_str(), O_RDWR | O_CREAT | O_EXCL, 0664)) {
//        // 文件存在
//        if (17 == errno) {
//            cout << "BPT::createTable  表已经存在" << std::endl;
//            return table;
//        }
//    }

    table->columnNums = columnNames.size();
    table->nextNew = 8192;
    table->columnNames = columnNames; // initializer_list 可以直接赋值给 vector
    table->hasIndexColumns = {};
    table->columnIndexRootOffsets = {};

    // 主键
    if(table->columnNames[0] == 'a'){
        table->hasIndexColumns.insert(table->hasIndexColumns.begin(),true);
        table->columnIndexRootOffsets.insert(table->columnIndexRootOffsets.begin(),8192);
        Node *genesisNode = new Node(LEAF_NODE, table->nextNew,table->columnNums,table->tableName);
        table->nextNew += genesisNode->getNodeSpaceSize();
        genesisNode->serialize();
    }else{
        cout << "Table::createTable 首列必须是 id！" << endl;
    }
    table->serialize();
    cout << "Table:: 序列化成功" << endl;
    return table;
}

Table* Table::useTable(string tableName){
    Table* table = nullptr;
    string tableFilePath = "../" + tableName;
    if (-1 == open(tableFilePath.c_str(), O_RDWR | O_CREAT | O_EXCL, 0664)) {
        // 文件存在
        if (17 == errno) {
            cout << "BPT::DESerialize  文件存在" << std::endl;
        }
        table = Table::deSerialize(tableName);
        cout << "BPT::DESerialize  反序列化成功" << std::endl;

    }else{
        cout << "Table::useTable 读取数据文件失败，请确保数据表已经创建" << std::endl;
    }
    return table;
}


void Table::serialize() {
    string tableFilePath = "../" + tableName;

    int fd = open(tableFilePath.c_str(), O_WRONLY | O_CREAT, 0664);
    if (-1 == fd) {
        perror("Table::serialize open");
        printf("errno = %d\n", errno);
    }

    if (-1 == lseek(fd, 0, SEEK_SET)) {
        perror("lseek");
    }
    auto *metaBuffer = (int64_t *) malloc(2048);
    auto *columnNameBuffer = (char *) malloc(1024);
    auto *hasIndexColumnBuffer = (bool *) malloc(1024);
    auto *columnIndexRootBuffer = (int64_t *) malloc(4096);

    int64_t p = 0;
    metaBuffer[p++] = this->columnNums;
    metaBuffer[p++] = this->nextNew;
    write(fd,metaBuffer,2048);

    p = 0;
    for(char columnName : this->columnNames) {
        columnNameBuffer[p++] = columnName;
    }

    write(fd,columnNameBuffer,1024);

    p = 0;
    for(bool hasIndex: this->hasIndexColumns){
        hasIndexColumnBuffer[p++] = hasIndex;
    }
    write(fd,hasIndexColumnBuffer,1024);

    p = 0;
    for(int64_t indexRootOffset: this->columnIndexRootOffsets){
        columnIndexRootBuffer[p++] = indexRootOffset;
    }
    write(fd,columnIndexRootBuffer,4096);

    if (-1 == close(fd)) {
        perror("close");
    }
    free(metaBuffer);
    free(columnNameBuffer);
    free(hasIndexColumnBuffer);
    free(columnIndexRootBuffer);
}


Table *Table::deSerialize(string tableName) {
    //  确保文件存在，即已经createTable。检测文件是否存在
    string tableFilePath = "../" + tableName;

    int fd = open(tableFilePath.c_str(), O_RDONLY);
    if (-1 == fd) {
        perror("Table::deSerialize open");
        printf("errno = %d\n", errno);
    }
    if (-1 == lseek(fd, 0, SEEK_SET)) {
        perror("lseek");
    }
    auto *metaBuffer = (int64_t *) malloc(2048);
    auto *columnNameBuffer = (char *) malloc(1024);
    auto *hasIndexColumnBuffer = (bool *) malloc(1024);
    auto *columnIndexRootBuffer = (int64_t *) malloc(4096);

    Table* table = new Table(tableName);
    int64_t p = 0;
    read(fd,metaBuffer,2048);
    table->columnNums = metaBuffer[p++];
    table->nextNew = metaBuffer[p++];


    read(fd,columnNameBuffer,1024);
    for(p = 0; p < table->columnNums; p++) {
        table->columnNames.push_back(columnNameBuffer[p]);
    }

    read(fd,hasIndexColumnBuffer,1024);
    for(p = 0; p < table->columnNums; p++) {
        table->hasIndexColumns.push_back(hasIndexColumnBuffer[p]);
    }

    read(fd,columnIndexRootBuffer,4096);
    for(p = 0; p < table->columnNums; p++) {
        table->columnIndexRootOffsets.push_back(columnIndexRootBuffer[p]);
    }


    if (-1 == close(fd)) {
        perror("close");
    }
    free(metaBuffer);
    free(columnNameBuffer);
    free(hasIndexColumnBuffer);
    free(columnIndexRootBuffer);
    return table;

}

Table::~Table() = default;