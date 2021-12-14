cmake_minimum_required(VERSION 3.10.2)
project(hw5)

add_executable(hw5
    src/main.cpp
)

set(CMAKE_CXX_STANDARD 20)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(hw5 PRIVATE Threads::Threads)
