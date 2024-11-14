cmake_minimum_required(VERSION 3.10)
project(MYOJ)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./output)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(OJ_server ./OJ_server/OJ_server.cc)
target_link_libraries(OJ_server PRIVATE jsoncpp ctemplate mysqlclient)

add_executable(compile_server ./compile_server/compile_server.cc)
target_link_libraries(compile_server PRIVATE jsoncpp)

add_executable(data_server ./data_server/data_server.cc)
target_link_libraries(data_server PRIVATE jsoncpp ctemplate mysqlclient)
