#include "../include/Table.h"
#include <sys/stat.h>
#include <dirent.h>

void printHelpMessage(){
    cout << "**************************************************************************************************************************"<<endl<<endl
         <<" 				                        Welcome to the ya_db" 	<< endl
         <<" 			                            Author: happytsing" << endl << endl
         <<"***************************************************************************************************************************"<<endl
         <<"  help                                                                                  print help message;"<<endl
         <<"  exit                                                                                  exit program;"<<endl
         <<"  SHOW TABLES            							                                    show tables;"<<endl
         <<"  CREATE TABLE <table_name> <column_name_1> ... <column_name_n>   	                 	create table;"<<endl
         <<"  INSERT INTO <table_name> <column_value_1> ... <column_value_n>                        insert record;"<<endl
         <<"  CREATE INDEX ON <table_name> <column_name>	                 	                    create index;"<<endl
         <<"  SELETE * FROM <table_name> WHERE <column_name> = {value} 				                search record;"<<endl
         <<"  SELETE * FROM <table_name> WHERE <column_name> IN {minValue} {maxValue})		        search records between (min, max);"<<endl
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
    cout << "finish in " + to_string(usedTime) + " seconds." << endl;
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
        } else {
            printErrorCommandMessage(command);
        }
        endTime = clock();
        printUsedTime(startTime, endTime);
        memset(command, 0, sizeof(command));
    }
}