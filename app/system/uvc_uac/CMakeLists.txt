add_subdirectory(st_uvc)

aux_source_directory(. DIR_APP_SRC)

include_directories(
	../sensor
	../isp
	../scl
	../venc
	../sys
	../common
	st_uvc
	#st_uac
	../../../soc_sdk/include/isp
)

link_directories(
	#st_uvc
	#st_uac
)

add_library(uvc_uac ${DIR_APP_SRC})

target_link_libraries(uvc_uac st_uvc)

