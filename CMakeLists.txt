cmake_minimum_required(VERSION 2.6)

project (gc)
set (gc_VERSION_MAJOR 2)
set (gc_VERSION_MINOR 2)
set (gc_VERSION_PATCH 0)
set (gc_VERSION "${gc_VERSION_MAJOR}.${gc_VERSION_MINOR}.${gc_VERSION_PATCH}")

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread)

include_directories(
    include
    ${Boost_INCLUDE_DIRS}
)

set (gc_SOURCES
    src/gc.cpp
    test/gc_main.cpp
    test/gc_test.cpp
)

add_executable(
    gc_test
    ${gc_SOURCES}
)

target_link_libraries(
    gc_test
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
)
