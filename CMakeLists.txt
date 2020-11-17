cmake_minimum_required(VERSION 3.13.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_STANDARD_REQUIRED ON)

project(FunctionalCxx)

find_package(Boost 1.65)

set(functional_cxx_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(base_path ${functional_cxx_INCLUDE_DIR}/functional)

message(STATUS "base_path = ${base_path}")

add_library(functional-cxx INTERFACE)
target_sources(functional-cxx INTERFACE ${base_path}/lazy-wrapper.hpp ${base_path}/stream.hpp ${base_path}/support/memory-hacks.hpp ${base_path}/support/unique-function.hpp)
target_include_directories(functional-cxx INTERFACE ${functional_cxx_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})

add_subdirectory(example)

