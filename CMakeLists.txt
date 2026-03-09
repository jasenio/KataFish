cmake_minimum_required(VERSION 3.16)
project(myengine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB ENGINE_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(myengine ${ENGINE_SOURCES})
target_include_directories(myengine PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_compile_options(myengine PRIVATE
  -O3
  -DNDEBUG
  -Wall
  -Wextra
  -pedantic-errors
  -pthread
)

target_link_options(myengine PRIVATE
  -static
  -static-libgcc
  -static-libstdc++
  -pthread
)
