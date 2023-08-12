aux_source_directory(. DIR_APP_SRC)

add_library(myfifo ${DIR_APP_SRC})

target_link_libraries(myfifo)

