

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src/transaction)


enable_language(ASM)

# exe path
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

set (SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set (PROJECT_THIRDPARTY_DIR ${SRC_DIR}/thirdparty)
set (PROJECT_PROTO_DIR ${SRC_DIR}/proto)
set (PROJECT_COMMON_DIR ${SRC_DIR}/common)


include_directories(${SRC_DIR})

# abseil-cpp
include_directories(${PROJECT_THIRDPARTY_DIR}/abseil-cpp/include)
include(${PROJECT_THIRDPARTY_DIR}/abseil-cpp/lib64/cmake/absl/abslConfig.cmake)


# proto
include_directories(${PROJECT_PROTO_DIR})
aux_source_directory(${PROJECT_PROTO_DIR} PROTO_CODE)

# common
include_directories(${PROJECT_COMMON_DIR})
aux_source_directory(${SRC_DIR}/common COMMON_CODE)


# coroutine
include_directories(${SRC_DIR}/coroutine)
aux_source_directory(${SRC_DIR}/coroutine COROUTINE_CODE)
# asm
FILE(GLOB_RECURSE SOURCES_S ${SRC_DIR}/coroutine/*.S)

# transaction
include_directories(${SRC_DIR}/transaction)
aux_source_directory(${SRC_DIR}/transaction TRANSACTION_CODE)

# timer
include_directories(${SRC_DIR}/timer)
aux_source_directory(${SRC_DIR}/timer TIMER_CODE)


# valgrind
include_directories(${PROJECT_THIRDPARTY_DIR}/valgrind)


# test
include_directories(${SRC_DIR}/test)


message("!!!!!!! CMAKE_SOURCE_DIR = " "${CMAKE_SOURCE_DIR}/src/transaction")


set(ALL_DIR ${PROTO_CODE} ${SOURCES_S} ${COMMON_CODE} ${COROUTINE_CODE} ${TRANSACTION_CODE} ${TIMER_CODE})

add_executable(main main.cpp ${ALL_DIR})



set(DEP_LIBS protobuf pthread)


target_link_libraries(main ${DEP_LIBS})

