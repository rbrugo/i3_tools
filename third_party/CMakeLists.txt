######################################################################
# @author      : Riccardo Brugo (brugo.riccardo@gmail.com)
# @file        : CMakeLists
# @created     : Saturday Apr 24, 2021 02:16:14 CEST
######################################################################
cmake_minimum_required(VERSION 3.16.2)

project(  )
add_executable(${CMAKE_PROJECT_NAME})

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

target_compile_features(${CMAKE_PROJECT_NAME} PUBLIC cxx_std_20)
find_package(Threads REQUIRED)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add rollbear as a library
add_library(rollbear INTERFACE)
target_include_directories(rollbear INTERFACE ${CMAKE_CURRENT_LIST_DIR}/rollbear)
