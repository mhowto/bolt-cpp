set(GSL_ROOT ${MAINFOLDER}/thirdparty/GSL)
set(GSL_INCLUDE_DIR ${GSL_ROOT}/include)
set(GSL_LIBRARIES GSL)
include_directories(${GSL_INCLUDE_DIR})

set(MOLLY_ROOT ${MAINFOLDER}/thirdparty/molly)
set(MOLLY_INCLUDE_DIR ${MOLLY_ROOT})
include_directories(${MOLLY_INCLUDE_DIR})

aux_source_directory(. DIR_LIB_SRCS)

add_library (boltcpp ${DIR_LIB_SRCS})
target_link_libraries(boltcpp molly)