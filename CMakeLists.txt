cmake_minimum_required(VERSION 3.0)

project(echo_server)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost COMPONENTS system thread REQUIRED)

add_executable(sserver sserver.cpp)

add_executable(aserver aserver.cpp)

target_link_libraries(sserver Boost::system Boost::thread)

target_link_libraries(aserver Boost::system Boost::thread)

