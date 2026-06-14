cmake_minimum_required(VERSION 3.14)

project(vsp-frame-snapshot-fix)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Threads REQUIRED)

include(FetchContent)
set (SUBHOOK_TESTS OFF CACHE BOOL "" FORCE)
set (SUBHOOK_INSTALL OFF CACHE BOOL "" FORCE)
set (SUBHOOK_STATIC ON CACHE BOOL "" FORCE)
FetchContent_Declare(
  subhook
  GIT_REPOSITORY https://github.com/wyrover/subhook
  GIT_TAG master
)
FetchContent_MakeAvailable(subhook)

add_library(vsp-frame-snapshot-fix SHARED src/plugin.cpp src/valve.cpp src/symbol.cpp)
target_include_directories(vsp-frame-snapshot-fix PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(vsp-frame-snapshot-fix PRIVATE subhook Threads::Threads ${CMAKE_DL_LIBS})
