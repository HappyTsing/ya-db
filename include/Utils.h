#ifndef YA_DB_UTILS_H
#define YA_DB_UTILS_H

#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <sstream>
#include "../include/Node.h"
#include "../include/BPT.h"

using namespace std;

#define LEAF_NODE_SIZE 4096
#define INTERNAL_NODE_SIZE 16384
#define BPT_META_OFFSET 0
#define BPT_META_SIZE 4096

namespace utils {
    // 从文件读取node
    Node *readNode(off_t offset, NODE_TYPE type) {
        int fd = open("../my.ibd", O_RDWR | O_CREAT);
        if (-1 == fd) {
            perror("open");
            printf("errno = %d\n", errno);
        }

        if (-1 == lseek(fd, offset, SEEK_SET)) {
            perror("lseek");
        }
        int bufferSize = INTERNAL_NODE_SIZE;
        if (type == LEAF_NODE) {
            bufferSize = LEAF_NODE_SIZE;
        }
        char *buffer = (char *) malloc(bufferSize);
        if (-1 == read(fd, buffer, bufferSize)) {
            perror("read");
        }

        if (-1 == close(fd)) {
            perror("close");
        }

        Node *node = Node::deSerialize(buffer);
        free(buffer);
        return node;
    }

    BPTMeta readBPTMeta() {
        int fd = open("../my.ibd", O_RDWR | O_CREAT); // TODO: 文件不存在时，特殊处理！
        if (-1 == fd) {
            perror("open");
            printf("errno = %d\n", errno);
        }

        if (-1 == lseek(fd, BPT_META_OFFSET, SEEK_SET)) {
            perror("lseek");
        }

        char buffer[BPT_META_SIZE] = {0};
        if (-1 == read(fd, buffer, BPT_META_SIZE)) {
            perror("read");
        }


        if (-1 == close(fd)) {
            perror("close");
        }

        BPTMeta meta = BPT::deSerialize(buffer);
        return meta;

    }

    bool writeNode(off_t offset, Node *node) {
        int fd = open("../my.ibd", O_RDWR | O_CREAT);
        if (-1 == fd) {
            perror("open");
            printf("errno = %d\n", errno);
            return false;
        }

        if (-1 == lseek(fd, offset, SEEK_SET)) {
            perror("lseek");
            return false;
        }

        int bufferSize = INTERNAL_NODE_SIZE;
        if (node->type == LEAF_NODE) {
            bufferSize = LEAF_NODE_SIZE;
        }
        char *buffer = (char *) malloc(bufferSize);
        node->serialize(buffer);
        if (-1 == write(fd, buffer, bufferSize)) {
            perror("write");
            return false;
        }
        if (-1 == close(fd)) {
            perror("close");
            return false;
        }
        free(buffer);
        return true;
    }

    off_t writeNewNode(Node *node) {
        int fd = open("../my.ibd", O_RDWR | O_APPEND);
        if (-1 == fd) {
            perror("open");
            printf("errno = %d\n", errno);
        }
        // 获取当前位置的偏移量
        off_t startOffset = lseek(fd, 0, SEEK_CUR);
        // 更新节点自身偏移量
        node->self = startOffset;
        if (-1 == startOffset) {
            perror("lseek");
        }

        int bufferSize = INTERNAL_NODE_SIZE;
        if (node->type == LEAF_NODE) {
            bufferSize = LEAF_NODE_SIZE;
        }
        char *buffer = (char *) malloc(bufferSize);
        node->serialize(buffer);
        if (-1 == write(fd, buffer, bufferSize)) {
            perror("write");
        }
        if (-1 == close(fd)) {
            perror("close");
        }
        free(buffer);
        return startOffset;
    }

    bool writeBPTMeta(BPT *bpt) {
        int fd = open("../my.ibd", O_RDWR | O_CREAT);
        if (-1 == fd) {
            perror("open");
            printf("errno = %d\n", errno);
            return false;
        }

        if (-1 == lseek(fd, BPT_META_OFFSET, SEEK_SET)) {
            perror("lseek");
            return false;
        }

        char buffer[BPT_META_SIZE] = {0};
        bpt->serialize(buffer);
        if (-1 == write(fd, buffer, BPT_META_SIZE)) {
            perror("read");
            return false;
        }
        if (-1 == close(fd)) {
            perror("close");
            return false;
        }
        return true;
    }


    vector<string> stringToVector(string str, char delim) {
        stringstream ss(str);
        vector<string> vector;
        string temp;
        while (getline(ss, temp, delim)) {
            if (temp != "") {
                vector.push_back(temp);
            }
        }
        return vector;
    }
}
#endif //YA_DB_UTILS_H
