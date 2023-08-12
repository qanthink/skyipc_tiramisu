add_subdirectory(myfifo)
add_subdirectory(myqueue)

aux_source_directory(myfifo DIR_APP_SRC)
aux_source_directory(myqueue DIR_APP_SRC)

add_library(mylib ${DIR_APP_SRC})

target_link_libraries(mylib myfifo myqueue)

