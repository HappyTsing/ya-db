cmake_minimum_required(VERSION 3.23)
project (YaSQL)

set(CMAKE_CXX_STANDARD 14)

add_executable(YaSQL src/main.cpp include/BPT.h src/BPT.cpp include/Node.h src/Node.cpp)
