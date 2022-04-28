aux_source_directory(. DIR_APP_SRC)

add_library(vpe ${DIR_APP_SRC})

include_directories(../sensor)

target_link_libraries(vpe 
	mi_vpe 
	mi_isp 
	cus3a
	cam_os_wrapper
)

