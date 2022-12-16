cmake_minimum_required(VERSION 3.20)
project(bookstore)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

include_directories(
        3rdparty/libull/include
)

set(src_files
        src/main.cpp
        src/user.cpp src/user.h)

add_executable(code ${src_files})

target_include_directories(code PUBLIC 3rdparty/libull/include)
target_link_directories(code PUBLIC 3rdparty/libull/lib/)
target_link_libraries(code ull)