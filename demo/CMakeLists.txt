


# exe path
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

set (DEMO_DIR ${PROJECT_SOURCE_DIR}/demo)
set (SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set (NETWORK_DIR ${PROJECT_SOURCE_DIR}/network)



# 执行 build.sh 脚本
execute_process(
    COMMAND bash "${DEMO_DIR}/proto/build.sh" "build"
    WORKING_DIRECTORY ${DEMO_DIR}/proto
)



include_directories(${SRC_DIR})
include_directories(${SRC_DIR}/proto)
include_directories(${SRC_DIR}/common)
include_directories(${SRC_DIR}/coroutine)
include_directories(${SRC_DIR}/transaction)
include_directories(${SRC_DIR}/timer)
# include_directories(${SRC_DIR}/thirdparty/abseil-cpp/include)

link_libraries(${PROJECT_SOURCE_DIR}/lib)

include_directories(${NETWORK_DIR})


# 获取项目根目录的绝对路径
get_filename_component(PROJECT_ROOT_DIR "${PROJECT_SOURCE_DIR}" ABSOLUTE)

# 转换为字符串宏（处理路径中的反斜杠）
string(REPLACE "\\" "\\\\" PROJECT_ROOT_MACRO "${PROJECT_ROOT_DIR}/")

# 添加编译定义
add_compile_definitions(PROJECT_ROOT_DIR="${PROJECT_ROOT_MACRO}")









include_directories(${DEMO_DIR})
include_directories(${DEMO_DIR}/proto)

aux_source_directory(${DEMO_DIR} SOURCES_S)
aux_source_directory(${DEMO_DIR}/proto SOURCES_PROTO)

set(ALL_DIR ${SOURCES_S} ${SOURCES_PROTO})

message("!!!!!!! CMAKE_SOURCE_DIR = " "${CMAKE_SOURCE_DIR}")

foreach(d ${ALL_DIR})
    message("File = " "${d}")
endforeach()


add_executable(demo main.cpp ${ALL_DIR})



set(DEP_LIBS protobuf pthread transaction tran_network)


target_link_libraries(demo ${DEP_LIBS})

