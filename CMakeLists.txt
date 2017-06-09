cmake_minimum_required (VERSION 2.8)

project (bolt_cpp)

set(MAINFOLDER ${PROJECT_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -stdlib=libc++")

add_subdirectory(thirdparty/GSL)
add_subdirectory(bolt)
add_subdirectory(thirdparty/googletest-release-1.8.0/googletest)

enable_testing()
add_subdirectory(test)

