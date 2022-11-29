#ifndef YA_DB_TABLE_H
#define YA_DB_TABLE_H

#include "../include/BPT.h"

using namespace std;
typedef vector<int64_t> Record;

class Table {

public:

    string tableName;                              // 表名
    int64_t columnNums;                            // 列的数量
    off64_t nextNew;                               // 新建节点时，其在文件中的偏移量
    vector<string> columnNames;                    // 列名
    vector<bool> hasIndexColumns;                  // 当前列是否创建索引
    vector<off64_t> columnIndexRootOffsets;        // 当前列对应的索引（即 B+ 树）的根节点的文件偏移量
    int primaryKeyId;  // TODO 从 1 开始自增。当不存在 id 时，将使用其作为聚集索引。当前并未使用，默认第一列必须是 id。

public:
    Table(string tableName, int64_t columnNums);

    virtual ~Table();

    static Table *createTable(string tableName, vector<string> columnNames);

    static Table *useTable(string tableName);

    bool createIndex(string columnName);

    bool insertRecord(Record record);

    void selectRecord(Key_t start, Key_t end, string columnName);

    void serialize();

    static Table *deSerialize(string);

    BPT *createBPT(off64_t root);


private:
    void doCreateIndex(Node *pNode, BPT *bpt, int lineNumber);

    int64_t getColumnNo(string columnName);

    void showRecord(vector<Record> recordList);
};

#endif //YA_DB_TABLE_H
