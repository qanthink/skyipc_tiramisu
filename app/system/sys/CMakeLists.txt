aux_source_directory(. DIR_APP_SRC)

add_library(sys ${DIR_APP_SRC})

include_directories(
	../vif
)

target_link_libraries(sys mi_sys)

