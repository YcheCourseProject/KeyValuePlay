cmake_minimum_required(VERSION 3.4)
project(KeyValueStore)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_executable(navie_store key_value_store.h test_naive_store.cpp)
add_subdirectory(malloc-test)