# changed target names to cmp0002
add_executable(sclient client.cpp)
add_executable(sserver server.cpp)

set_property(TARGET sclient PROPERTY OUTPUT_NAME client)
set_property(TARGET sserver PROPERTY OUTPUT_NAME server)

target_link_libraries(sclient Boost::system Boost::thread)
target_link_libraries(sserver Boost::system Boost::thread)