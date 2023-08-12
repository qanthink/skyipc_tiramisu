aux_source_directory(. DIR_APP_SRC)

include_directories(
	../sensor
	../sys
	../common
)

add_library(vif ${DIR_APP_SRC})

target_link_libraries(vif mi_vif)

