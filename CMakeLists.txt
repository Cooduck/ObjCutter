cmake_minimum_required(VERSION 3.28)
project(ObjCutter)

# 指定了项目使用的 C++ 标准为 C++20
set(CMAKE_CXX_STANDARD 20)      

add_executable(ObjCutter main.cpp
        Const.h
        Types.cpp
        Types.h
        ObjCutter.cpp
        ObjCutter.h
)
