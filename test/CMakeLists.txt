set(GTEST_ROOT ${MAINFOLDER}/thirdparty/googletest-release-1.8.0/googletest)

set(GTEST_INCLUDE_DIR ${GTEST_ROOT}/include)
set(GTEST_LIBRARIES gtest gtest_main)

set(GSL_ROOT ${MAINFOLDER}/thirdparty/GSL)
set(GSL_INCLUDE_DIR ${GSL_ROOT}/include)
set(GSL_LIBRARIES GSL)
include_directories(${GSL_INCLUDE_DIR})

include_directories(${GTEST_INCLUDE_DIR})
include_directories(${MAINFOLDER})

aux_source_directory(. DIR_LIB_SRCS)
add_executable(testall ${DIR_LIB_SRCS} ${GTEST_ROOT}/src/gtest_main.cc)
target_link_libraries(testall boltcpp gtest gtest_main)

#ADD_EXECUTABLE(TFoo test_bolt_unix.cpp ${GTEST_ROOT}/src/gtest_main.cc)

add_test(NAME MyTest COMMAND testall)
