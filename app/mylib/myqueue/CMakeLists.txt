aux_source_directory(. DIR_APP_SRC)

add_library(myqueue ${DIR_APP_SRC})

target_link_libraries(myqueue)

