cmake_minimum_required(VERSION 3.23)
project (ya-db)

set(CMAKE_CXX_STANDARD 14)

add_executable(ya-db src/main.cpp include/BPT.h src/BPT.cpp include/Node.h src/Node.cpp include/Table.h src/Table.cpp)
#add_executable(ya-db src/main.cpp include/Node.h src/Node.cpp)
#add_executable(ya-db src/main.cpp)
