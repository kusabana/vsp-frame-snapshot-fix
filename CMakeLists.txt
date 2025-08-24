cmake_minimum_required(VERSION 3.5)

project(vsp-frame-snapshot-fix)

set (CMAKE_CXX_FLAGS "-m32")
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(vsp-frame-snapshot-fix SHARED src/plugin.cpp src/valve.cpp src/symbol.cpp)
target_include_directories(vsp-frame-snapshot-fix PUBLIC ${PROJECT_SOURCE_DIR}/include)

