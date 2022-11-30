#include "../include/Table.h"

void testTableSerialize() {
    std::cout << "Test Table Serialize" << std::endl;
    Table *table = new Table("testTableSerialize", 4);
    table->columnNums = 4;
    table->nextNew = 8192;
    table->hasIndexColumns.push_back(true);
    table->hasIndexColumns.push_back(true);
    table->hasIndexColumns.push_back(true);
    table->columnNames.push_back("id");
    table->columnNames.push_back("one");
    table->columnNames.push_back("two");
    table->columnNames.push_back("three");
    table->columnIndexRootOffsets.push_back(1024);
    table->columnIndexRootOffsets.push_back(2048);
    table->columnIndexRootOffsets.push_back(3072);
    table->columnIndexRootOffsets.push_back(4096);

    table->serialize();
    Table *table_read = Table::deSerialize("testTableSerialize");
    cout << table_read->nextNew << std::endl;
    cout << table_read->columnNames[0] << std::endl;
}

void testTable() {
    std::cout << "Test Table" << std::endl;
    Table *table = Table::useTable("test");
//    Table *table = Table::createTable("test",{"id","one","two","three"});
    srand((unsigned) time(NULL)); // random seed
    for (int i = 1; i <= 10; i++) {
        std::cout << "======================================" << std::endl;
        int x = rand() % 10000000000 + 1;
//        int x = i;
        std::cout << "Main:testBTP: 插入第：" << i << "个数据" << std::endl;
        Record record;
        record.push_back(x);
        record.push_back(x);
        record.push_back(x);
        record.push_back(x);
        table->insertRecord(record);
    }
    std::cout << "=============结束插入=================" << std::endl;

    std::cout << "************** 打印主键索引树 *************" << std::endl;
    BPT *primaryKeyBPT = table->createBPT(table->columnIndexRootOffsets[0]);
    primaryKeyBPT->printTree();
    std::cout << "**************** 打印完毕 ****************" << std::endl;

    std::cout << "**************** 创建索引 ****************" << std::endl;
    table->createIndex("one");
    std::cout << "************** 创建索引完毕 ***************" << std::endl;

    std::cout << "*************** 打印索引树 ***************" << std::endl;
    BPT *indexBPT = table->createBPT(table->columnIndexRootOffsets[1]);
    indexBPT->printTree();
    std::cout << "**************** 打印完毕 ****************" << std::endl;

    std::cout << "**************** 搜索数据 ****************" << std::endl;
    table->selectRecord(1, 100000000, "one");
    std::cout << "*************** 搜索数据结束 **************" << std::endl;

}

void testNodeSerialize() {
    std::cout << "Test Node Serialize" << std::endl;
    Node *node = new Node(LEAF_NODE, 1024, "testNodeSerialize");
    node->father = 2048;
    node->rightBrother = 4096;
    node->self = 1024;
    node->keyNums = 2;
    node->keys[0] = 11;
    node->keys[1] = 22;
    node->children[0] = 1111;
    node->children[1] = 2222;
    node->children[2] = 3333;
    vector<int64_t> record;
    record.push_back(1);
    record.push_back(2);
    record.push_back(4);
    record.push_back(5);
    node->values[0] = record;
    node->values[1] = record;
    node->serialize();
    Node *d_node = Node::deSerialize(node->self, "testNodeSerialize");
    d_node->printNode();
}

int testRW() {
    std::cout << "Test Read/Write" << std::endl;
    std::cout << "***** 打开文件 *****" << std::endl;
    int fd = open("../rwTest", O_RDWR | O_CREAT, 0664);

    std::cout << "***** 写入文件 *****" << std::endl;
    lseek(fd, 0, SEEK_SET);

    // 写入数字
    auto *intBuffer = (int64_t *) malloc(2048);
    intBuffer[0] = 1234;
    intBuffer[1] = 5678;
    write(fd, intBuffer, 2048);


    // 写入 string
    string line1 = "id";
    string line2 = "one";
    string line3 = "two";
    string line4 = "three";
    string full = line1 + "#" + line2 + "#" + line3 + "#" + line4;
    char *columnNameBuffer = (char *) full.c_str();
    write(fd, columnNameBuffer, 2048);

    std::cout << "***** 读出文件 *****" << std::endl;
    lseek(fd, 0, SEEK_SET);

    // 读出 int
    auto *intBuffer_read = (int64_t *) malloc(2048);
    read(fd, intBuffer_read, 2048);
    std::cout << intBuffer_read[0] << std::endl;
    std::cout << intBuffer_read[1] << std::endl;

    // 读出 string
    auto *columnNameBuffer_read = (char *) malloc(2048);
    read(fd, columnNameBuffer_read, 2048);
    char pattern = '#';
    char *temp = strtok(columnNameBuffer_read, &pattern);
    while (temp != NULL) {
        cout << temp << endl;
        temp = strtok(NULL, &pattern);
    }

    if (-1 == close(fd)) {
        perror("close");
    }

}

int main() {
    testTableSerialize();
    testTable();
    testNodeSerialize();
    testRW();
}