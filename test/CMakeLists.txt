set(GTEST_ROOT ${MAINFOLDER}/thirdparty/googletest/googletest)
set(GTEST_INCLUDE_DIR ${GTEST_ROOT}/include)
include_directories(${GTEST_INCLUDE_DIR})

set(GSL_ROOT ${MAINFOLDER}/thirdparty/GSL)
set(GSL_INCLUDE_DIR ${GSL_ROOT}/include)
include_directories(${GSL_INCLUDE_DIR})

set(MOLLY_ROOT ${MAINFOLDER}/thirdparty/molly)
set(MOLLY_INCLUDE_DIR ${MOLLY_ROOT})
include_directories(${MOLLY_INCLUDE_DIR})

include_directories(${MAINFOLDER})

aux_source_directory(. DIR_LIB_SRCS)
add_executable(test_bolt_all ${DIR_LIB_SRCS} ${GTEST_ROOT}/src/gtest_main.cc)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_link_libraries(test_bolt_all boltcpp molly gtest gtest_main c++experimental)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_link_libraries(test_bolt_all boltcpp molly gtest gtest_main stdc++fs)
else()
    message(SEND_ERROR "You are using an unsupported compiler! (Not G++ or clang++)")
endif()

add_test(NAME MyTest COMMAND test_bolt_all)
