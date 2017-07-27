cmake_minimum_required (VERSION 2.8)

project (bolt_cpp)

enable_testing()

option(BUILD_BOLTCPP_TEST "Builds the boltcpp test subproject" OFF)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -stdlib=libc++ -Wall")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall")
else()
    message(SEND_ERROR "You are using an unsupported compiler! (Not G++ or clang++)")
endif()

set(MAINFOLDER ${PROJECT_SOURCE_DIR})
add_subdirectory(bolt)
add_subdirectory(thirdparty/molly)

if (NOT TARGET gtest)
    add_subdirectory(thirdparty/googletest/googletest)
endif()

if (BUILD_BOLTCPP_TEST)
    add_subdirectory(test)
endif()