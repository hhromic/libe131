cmake_minimum_required(VERSION 3.13)
project(examples C)

add_executable(test_client test_client.c)
add_executable(test_mcast_client test_mcast_client.c)
add_executable(test_server test_server.c)

target_link_libraries(test_client libe131)
target_link_libraries(test_mcast_client libe131)
target_link_libraries(test_server libe131)
