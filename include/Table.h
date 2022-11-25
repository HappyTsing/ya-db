#ifndef YA_DB_TABLE_H
#define YA_DB_TABLE_H
#include <string>
#include <initializer_list>
#include <vector>

using namespace std;
typedef vector<int64_t> Record;
class Table{
public:
    // 表名
    string tableName;
    // 列的数量
    int columnNums;
    vector<string> columns;

    // 记录的数量，默认为0，从1开始自增。将自动以这个作为索引key。
    int recordNums_indexId;
public:
    // 默认列为 int64_t 类型
    Table(string tableName, initializer_list<string> columnNames);
    virtual ~Table();
    bool createIndex(string columnName);                 // 创建索引，首先判断索引是否存在
    bool insertRecord(Record record); // 判断输入的列的长度是否等于 columnNums；
//    vector<Record> selectRecords()  // 搜索
};



#endif //YA_DB_TABLE_H
