aux_source_directory(. DIR_APP_SRC)

add_library(aad ${DIR_APP_SRC})

target_link_libraries(aad faad)

