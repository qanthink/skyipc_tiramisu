aux_source_directory(. DIR_APP_SRC)

add_library(venc ${DIR_APP_SRC})

target_link_libraries(venc mi_venc)

