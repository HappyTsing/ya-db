#ifndef YA_DB_TABLE_H
#define YA_DB_TABLE_H
#include <string>
#include <initializer_list>
#include <vector>
#include "../include/BPT.h"
using namespace std;
typedef vector<int64_t> Record;
class Table{

public:
    // 表名
    string tableName;
    // 列的数量
    int64_t columnNums;
    off64_t nextNew;        // 下一个插入的节点偏移量
    vector<char> columnNames;
    vector<bool> hasIndexColumns;
    vector<off64_t> columnIndexRootOffsets;
    BPT * bpt;  //  表的数据存储


    // 记录的数量，默认为0，从1开始自增。将自动以这个作为索引key。
    int recordNums_indexId;

public:
    // 默认列为 int64_t 类型
    Table(string tableName);
    virtual ~Table();
    static Table* createTable(string tableName, initializer_list<char> columnNames);
    static Table* useTable(string tableName);


    bool createIndex(char columnName);                 // 创建索引，首先判断索引是否存在
    bool insertRecord(Record record); // 判断输入的列的长度是否等于 columnNums；
    vector<Record> selectRecords();  // 搜索

    void serialize();
    static Table* deSerialize(string);

private:
    BPT* createBPT(off64_t root);
};

#endif //YA_DB_TABLE_H
