aux_source_directory(. DIR_APP_SRC)

include_directories(
	../sensor
	../isp
	../common
)

add_library(scl ${DIR_APP_SRC})

target_link_libraries(scl mi_scl)

