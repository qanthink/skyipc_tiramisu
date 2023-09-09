aux_source_directory(. DIR_APP_SRC)

include_directories(
)

add_library(st_uvc ${DIR_APP_SRC})

target_link_libraries(st_uvc)

