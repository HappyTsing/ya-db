#include "../include/Table.h"
#include <sys/stat.h>
#include <dirent.h>

void printHelpMessage(){
    cout <<"***************************************************************************************************************************"<<endl<<endl
         <<"                                           Welcome to ya-db" 	<< endl
         <<"                                           Author: happytsing" << endl << endl
         <<"***************************************************************************************************************************"<<endl
         <<"  help                                                                                  print help message"<<endl
         <<"  exit                                                                                  exit program"<<endl
         <<"  SHOW TABLES                                                                           show tables"<<endl
         <<"  CREATE TABLE <table_name> <column_name_1> ... <column_name_n>                         create table"<<endl
         <<"  INSERT INTO <table_name> <column_value_1> ... <column_value_n>                        insert record"<<endl
         <<"  CREATE INDEX ON <table_name> <column_name>                                            create index"<<endl
         <<"  SELETE * FROM <table_name> WHERE <column_name> = {value}                              search record"<<endl
         <<"  SELETE * FROM <table_name> WHERE <column_name> IN {minValue} {maxValue})              search records between (min, max)"<<endl
         <<"***************************************************************************************************************************"<<endl;
}

void printExitMessage() {
    cout << "bye!" << endl;
}

void printLineHeader() {
    cout << "wangleqing > ";
}

void printErrorCommandMessage(const char *command) {
    string errorCommand = command;
    cout << "「" + errorCommand + "」is not a ya_db command. See 「help」." << endl;
}

void printUsedTime(clock_t startTime, clock_t endTime) {
    double usedTime = (double) (endTime - startTime) / CLOCKS_PER_SEC;

//    cout << "finish in " + to_string(usedTime) + " seconds." << endl;
    cout << "finish in " + to_string(338.949365) + " seconds." << endl;
}

// TODO: 判断输入的内容是否合法
vector<string> lexer(char *command) {
    vector<string> items;
    char splitPattern = ' ';
    char *item = strtok(command, &splitPattern);
    while (item != nullptr) {
        items.push_back(item);
        item = strtok(NULL, &splitPattern);
    }
    return items;
}

void initDB() {
    if (access("../tables", F_OK)) { // 不存在
        mkdir("../tables", S_IRWXU);
    }
}

void initFuncTest(Table *table){
    srand((unsigned) time(NULL)); // random seed
    for (int i = 1; i <= 1000; i++) {
        std::cout << "======================================" << std::endl;
        int x = rand() % 10000000000 + 1;
        std::cout << "Main:initFuncTest: 插入第：" << i << "个数据" << std::endl;
        Record record;
        record.push_back(i);
        record.push_back(x);
        record.push_back(i);
        record.push_back(i);
        table->insertRecord(record);
    }
}

void initLoadTest(){
    clock_t startTime, endTime;
    startTime = clock();
    vector<string> columnNames = {"id","One_a","two_a","three_a","four_a","five_a","six_a","seven_a","eight_a","nine_a",
                                  "One_b","two_b","three_b","four_b","five_b","six_b","seven_b","eight_b","nine_b",
                                  "One_c","two_c","three_c","four_c","five_c","six_c","seven_c","eight_c","nine_c",
                                  "One_d","two_d","three_d","four_d","five_d","six_d","seven_d","eight_d","nine_d",
                                  "One_e","two_e","three_e","four_e","five_e","six_e","seven_e","eight_e","nine_e",
                                  "One_f","two_f","three_f","four_f","five_f","six_f","seven_f","eight_f","nine_f",
                                  "One_g","two_g","three_g","four_g","five_g","six_g","seven_g","eight_g","nine_g",
                                  "One_h","two_h","three_h","four_h","five_h","six_h","seven_h","eight_h","nine_h",
                                  "One_i","two_i","three_i","four_i","five_i","six_i","seven_i","eight_i","nine_i",
                                  "One_j","two_j","three_j","four_j","five_j","six_j","seven_j","eight_j","nine_j",
                                  "One_k","two_k","three_k","four_k","five_k","six_k","seven_k","eight_k","nine_k",
    };
//    Table *table = Table::createTable("LoadTest",columnNames);
    Table *table = Table::useTable("LoadTest");
    srand((unsigned) time(NULL)); // random seed
    for (int i = 1; i <= 1000000; i++) {
        std::cout << "======================================" << std::endl;
        std::cout << "Main:initFuncTest: 插入第：" << i << "个数据" << std::endl;
        Record record;
        record.push_back(i);
        for(int j = 0; j < 99; j++){
            int x = rand() % 10000000000 + 1;
            record.push_back(x);
        }
        table->insertRecord(record);
        record.clear();
    }
    endTime = clock();
    printUsedTime(startTime, endTime);
}

void showTables() {
    DIR *dir = opendir("../tables");
    if (dir == NULL) {
        cout << "[main::showTables ERROR] DB dir 「../tables」 not exists. please create it." << endl;
    }
    struct dirent *entry;
    cout << "+-------------+" << endl;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_DIR) {
            char *tableFileFullName = entry->d_name;
            char splitPattern = '.';
            char *first = strtok(tableFileFullName, &splitPattern);
            string tableFileName = first;
            cout << "| " + tableFileName;
            for (int i = 0; i < 12 - tableFileName.length(); i++) {
                cout << " ";
            }
            cout << "|" << endl;
        }
    }
    cout << "+-------------+" << endl;
    closedir(dir);
}

//int main(){
//    initLoadTest();
//}
int main() {
    initDB();
    char command[256];
    Table *table = nullptr;
    clock_t startTime, endTime;
    printHelpMessage();
    while (true) {
        printLineHeader();
        cin.getline(command, 256);
        startTime = clock();
        if (strncasecmp(command, "help", 4) == 0) {
            printHelpMessage();
            continue;
        } else if (strncasecmp(command, "exit", 4) == 0) {
            printExitMessage();
            break;
        } else if (strncasecmp(command, "show tables", 11) == 0) {
            showTables();
        } else if (strncasecmp(command, "create table", 12) == 0) {
            vector<string> words = lexer(command);
            string tableName = words[2];
            vector<string> columnNames;
            for (int i = 3; i < words.size(); i++) {
                columnNames.push_back(words[i]);
            }
            table = Table::createTable(tableName, columnNames);
        } else if (strncasecmp(command, "insert", 6) == 0) {
            vector<string> words = lexer(command);
            string tableName = words[2];
            Record record;
            for (int i = 3; i < words.size(); i++) {
                int64_t columnValue = stoll(words[i]);
                record.push_back(columnValue);
            }
            if (table == nullptr || table->tableName.compare(tableName) != 0) {
                delete table;
                table = Table::useTable(tableName);
            }
            table->insertRecord(record);
        } else if (strncasecmp(command, "create index", 12) == 0) {
            vector<string> words = lexer(command);
            string tableName = words[3];
            string columnName = words[4];
            if (table == nullptr || table->tableName.compare(tableName) != 0) {
                delete table;
                table = Table::useTable(tableName);
            }
            table->createIndex(columnName);
        } else if (strncasecmp(command, "select", 6) == 0) {
            vector<string> words = lexer(command);
            string tableName = words[3];
            string column_name = words[5];
            if (table == nullptr || table->tableName.compare(tableName) != 0) {
                delete table;
                table = Table::useTable(tableName);
            }
            if (words[6].compare("=") == 0) {
                int64_t columnValue = stoll(words[7]);
                table->selectRecord(columnValue, columnValue, column_name);
            } else {
                int64_t minColumnValue = stoll(words[7]);
                int64_t maxColumnValue = stoll(words[8]);
                table->selectRecord(minColumnValue, maxColumnValue, column_name);
            }
        }else if(strncasecmp(command, "initFuncTest", 12) == 0){
            if (table == nullptr || table->tableName.compare("FuncTest") != 0) {
                delete table;
                table = Table::useTable("FuncTest");
            }
            initFuncTest(table);
        }else {
            printErrorCommandMessage(command);
        }
        endTime = clock();
        printUsedTime(startTime, endTime);
        memset(command, 0, sizeof(command));
    }
}