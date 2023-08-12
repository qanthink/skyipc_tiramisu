aux_source_directory(. DIR_APP_SRC)

include_directories(
	../common
)

add_library(scl ${DIR_APP_SRC})

target_link_libraries(scl mi_scl)

