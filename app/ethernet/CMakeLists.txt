aux_source_directory(. DIR_APP_SRC)

include_directories(
	../spipanel
)

add_library(ethernet ${DIR_APP_SRC})

target_link_libraries(ethernet)

