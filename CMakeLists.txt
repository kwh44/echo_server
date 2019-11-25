cmake_minimum_required(VERSION 3.0)

project(echo_server)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Release)

find_package(Boost REQUIRED COMPONENTS system thread)

set(Boost_USE_MULTITHREADED ON)

include_directories(${Boost_INCLUDE_DIRS})

set(ADDITIONAL_LIBRARIES "")

if(MINGW)
    set(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} -lws2_32 -lwsock32)
elseif(MSVC)
    set(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} ws2_32.lib wsock32.lib)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_WIN32_WINNT=0x0601 /D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING")
endif()

add_executable(aserver aserver.cpp)
add_executable(cserver cserver.cpp)

target_link_libraries(cserver ${Boost_LIBRARIES} ${ADDITIONAL_LIBRARIES})
target_link_libraries(aserver ${Boost_LIBRARIES} ${ADDITIONAL_LIBRARIES})