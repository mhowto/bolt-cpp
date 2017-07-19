cmake_minimum_required (VERSION 2.8)

project (bolt_cpp)

set(MAINFOLDER ${PROJECT_SOURCE_DIR})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -stdlib=libc++ -Wall")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall")
else()
    message(SEND_ERROR "You are using an unsupported compiler! (Not G++ or clang++)")
endif()

#add_subdirectory(thirdparty/GSL)
add_subdirectory(bolt)
add_subdirectory(thirdparty/googletest-release-1.8.0/googletest)

enable_testing()
add_subdirectory(test)