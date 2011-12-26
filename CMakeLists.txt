cmake_minimum_required(VERSION 2.6)

project (gc)
set (gc_VERSION_MAJOR 0)
set (gc_VERSION_MINOR 1)
set (gc_VERSION_PATCH 0)
set (gc_VERSION "${gc_VERSION_MAJOR}.${gc_VERSION_MINOR}.${gc_VERSION_PATCH}")

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread)

include_directories(
	src
	${Boost_INCLUDE_DIRS}
)

set (gc_SOURCES
	test/gc_main.cpp
	test/gc_test.cpp
)

if (APPLE)
    add_definitions(-D_MAC)
elseif (WIN32)
    add_definitions(-D_WIN32)
elseif (UNIX)
    add_definitions(-DLINUX)
elseif (CMAKE_SYSTEM_NAME MATCHES "(Solaris|SunOS)")
    add_definitions(-D__sun)
endif()

if (CMAKE_SYSTEM_PROCESSOR MATCHES "sparc")
    add_definitions(-D__sparc__)
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc")
    add_definitions(-D__powerpc__)
endif()

add_executable(
	gc_test
    ${gc_SOURCES}
)

target_link_libraries(
    gc_test
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
)
