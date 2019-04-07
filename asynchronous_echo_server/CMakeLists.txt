add_executable(aclient client.cpp)
add_executable(aserver server.cpp)

set_property(TARGET aclient PROPERTY OUTPUT_NAME client)
set_property(TARGET aserver PROPERTY OUTPUT_NAME server)

target_link_libraries(aclient Boost::system Boost::thread)
target_link_libraries(aserver Boost::system Boost::thread)