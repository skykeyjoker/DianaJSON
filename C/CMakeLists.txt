cmake_minimum_required(VERSION 3.8)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


project(DianaJsonC)

add_executable(DianaJsonC dianajson.h dianajson.c simpletest.c)