project(calculator)
cmake_minimum_required(VERSION 3.15)

if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
else()
  add_compile_options("/std:c++17")
  set_property(GLOBAL PROPERTY USE_FOLDERS On)
endif()

set(SOURCES main.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})