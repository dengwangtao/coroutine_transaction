

aux_source_directory(. SRC_LIST)

add_library(tran_network STATIC ${SRC_LIST})

# target_link_libraries(dwt_muduo pthread)