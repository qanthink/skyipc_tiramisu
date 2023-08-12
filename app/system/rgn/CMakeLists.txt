add_subdirectory(st_rgn)

aux_source_directory(. DIR_APP_SRC)

include_directories(
	st_rgn
	../vpe
	../common
)

link_directories(
	st_rgn
)

add_library(rgn ${DIR_APP_SRC})

target_link_libraries(rgn st_rgn mi_rgn)

