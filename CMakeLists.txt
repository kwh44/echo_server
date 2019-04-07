cmake_minimum_required(VERSION 3.0)

project(echo_server)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost COMPONENTS system thread REQUIRED)

add_subdirectory(synchronous_echo_server sync_echo_server)

add_subdirectory(asynchronous_echo_server async_echo_server)
