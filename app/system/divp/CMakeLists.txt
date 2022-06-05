aux_source_directory(. DIR_APP_SRC)

include_directories(../venc)

add_library(divp ${DIR_APP_SRC})

target_link_libraries(divp mi_divp)

