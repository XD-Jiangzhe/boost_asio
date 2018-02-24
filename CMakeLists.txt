cmake_minimum_required(VERSION 3.5)
project(posix_time)

set(CMAKE_CXX_STANDARD 14)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../bin)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++14 -pthread")

find_library(protobuf protobuf)
message("${protobuf}")

add_executable(posix_time main.cc)
target_link_libraries(posix_time boost_system)

add_executable(bind bind.cc)
target_link_libraries(bind boost_system)

add_executable(printer printer.cc)
target_link_libraries(printer boost_system)

add_executable(daytime daytime.cc)
target_link_libraries(daytime  boost_system pthread)

add_executable(multithread multithread.cc)
target_link_libraries(multithread boost_system pthread)

add_executable(daytimeserver daytimeserver.cc)
target_link_libraries(daytimeserver boost_system pthread)

add_executable(asyncdaytimeserver asyncdaytimeserver.cc)
target_link_libraries(asyncdaytimeserver boost_system pthread)

add_executable(test_1 test.cc)

add_executable(echoserver echoserver.cc)
target_link_libraries(echoserver boost_system pthread)

add_executable(echoclient echoclient.cc)
target_link_libraries(echoclient boost_system pthread)

add_executable(chat_server char_server.cc)
target_link_libraries(chat_server boost_system pthread)

add_executable(chat_client chat_client.cc) 
target_link_libraries(chat_client boost_system pthread)

add_executable(struct_header_client struct_header_client.cc) 
target_link_libraries(struct_header_client boost_system boost_serialization ${protobuf} pthread)

add_executable(struct_header_server struct_header_server.cc) 
target_link_libraries(struct_header_server boost_system boost_serialization ${protobuf} pthread)

add_executable(serilizationServer serilizationServer.cc) 
target_link_libraries(serilizationServer boost_system boost_serialization  ${protobuf} pthread)

add_executable(serilizationClient serilizationClient.cc) 
target_link_libraries(serilizationClient boost_system boost_serialization ${protobuf} pthread)

add_executable(JsonObjectserver JsonObjectserver.cc) 
target_link_libraries(JsonObjectserver boost_system boost_serialization ${protobuf} pthread)

add_executable(Json_client Json_client.cc) 
target_link_libraries(Json_client boost_system boost_serialization ${protobuf} pthread)

add_executable(Protocal_client   Protocal_client.cc Protocal.pro.pb.cc Protocal.pro.pb.h) 
target_link_libraries(Protocal_client boost_system boost_serialization ${protobuf} pthread)

add_executable(Protocal_server  Protocal_server.cc Protocal.pro.pb.cc Protocal.pro.pb.h) 
target_link_libraries(Protocal_server boost_system boost_serialization ${protobuf} pthread)

add_executable(multithread_protocal  multithread_protocal.cc Protocal.pro.pb.cc Protocal.pro.pb.h) 
target_link_libraries(multithread_protocal boost_system boost_serialization ${protobuf} pthread)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_LOG_DYN_LINK")
add_executable(multithread_protocal_log  multithread_protocal_log.cc Protocal.pro.pb.cc Protocal.pro.pb.h) 
target_link_libraries(multithread_protocal_log boost_system boost_thread boost_log boost_log_setup boost_serialization ${protobuf} pthread )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_LOG_DYN_LINK")
add_executable(multithread_pro_copy_on_write  multithread_pro_copy_on_write.cc Protocal.pro.pb.cc Protocal.pro.pb.h) 
target_link_libraries(multithread_pro_copy_on_write boost_system boost_thread boost_log boost_log_setup boost_serialization ${protobuf} pthread )