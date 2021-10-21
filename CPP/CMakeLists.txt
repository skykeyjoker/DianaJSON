cmake_minimum_required(VERSION 3.8)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(DianaJsonCPP)

add_executable(DianaJsonCPP json.h json.cpp jsonvalue.h jsonvalue.cpp jsonerr.h parse.h parse.cpp test.cpp)