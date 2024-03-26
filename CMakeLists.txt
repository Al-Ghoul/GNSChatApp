cmake_minimum_required(VERSION 3.27.8)

project(example_chat CXX)

set(SOURCE_FILES_DIR "../src")
set(SOURCE_FILES ${SOURCE_FILES_DIR}/main.cc ${SOURCE_FILES_DIR}/utils.cc
                 ${SOURCE_FILES_DIR}/server.cc ${SOURCE_FILES_DIR}/client.cc)
set(CMAKE_DEBUG_POSTFIX d)

add_executable(example_chat ${SOURCE_FILES})
set_target_properties(example_chat PROPERTIES DEBUG_POSTFIX
                                              ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(example_chat GameNetworkingSockets)
