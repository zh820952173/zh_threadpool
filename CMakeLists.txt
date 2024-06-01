cmake_minimum_required(VERSION 3.20)

project(threadpool LANGUAGES CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)

file(GLOB_RECURSE SRC ${
  CMAKE_SOURCE_DIR}/*.cc)
set(CMAKE_CXX_STANDARD 20)

add_executable(threadpool_demo ${SRC})
target_include_directories(threadpool_demo PRIVATE ${CMAKE_SOURCE_DIR}/threadpool)
target_link_libraries(threadpool_demo PRIVATE pthread)
set_target_properties(threadpool_demo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)