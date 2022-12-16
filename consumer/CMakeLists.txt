cmake_minimum_required(VERSION 3.24)
project(consumer)

set(CMAKE_CXX_STANDARD 17)

add_executable(consumer consumer.cpp commodity_names.h)
