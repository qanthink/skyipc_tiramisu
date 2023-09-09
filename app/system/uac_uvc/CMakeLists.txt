add_subdirectory(st_uac)
add_subdirectory(st_uvc)

aux_source_directory(. DIR_APP_SRC)

include_directories(
	../sensor
	../isp
	#../../../soc_sdk/include/isp
	../scl
	../venc
	../sys
	../common
	../audio_ai
	st_uac
	st_uvc
)

link_directories(
	#st_uvc
	#st_uac
)

add_library(uac_uvc ${DIR_APP_SRC})

target_link_libraries(uac_uvc st_uac st_uvc)

