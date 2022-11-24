#include <iostream>
#include "../include/Table.h"
#include "../include/BPT.h"


// 初始化的时候新建一个文件夹。 TODO 抽象出一个CreateTable()的方法
Table::Table(string tableName,initializer_list<string> columnNames){
    this->tableName = tableName;
    this->columnNums = columnNames.size();
    std::cout << "TableName: " << tableName << std::endl;
    std::cout << "LineNums: " << columnNums << std::endl;
    this->columns = columnNames; // initializer_list 可以直接赋值给 vector

}

bool Table::createIndex(string columnName) {
    return false;
}

bool Table::insertRecord(Record record){
    return false;
}

Table::~Table() = default;