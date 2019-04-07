add_executable(client client.cpp)

add_executable(server server.cpp)

target_link_libraries(client Boost::system Boost::thread)
target_link_libraries(server Boost::system Boost::thread)