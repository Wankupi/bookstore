cmake_minimum_required(VERSION 3.20)
project(bookstore)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

set(src_files
        src/user/User.cpp
        src/book/Book.cpp
        src/finance/Finance.cpp
        src/store/StoreBranch.cpp
        src/main.cpp
        )

add_executable(code ${src_files})
