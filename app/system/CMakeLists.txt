add_subdirectory(sensor)
add_subdirectory(vif)
add_subdirectory(isp)
add_subdirectory(scl)
#add_subdirectory(divp)
add_subdirectory(venc)
add_subdirectory(sys)
add_subdirectory(common)
add_subdirectory(audio_ai)
add_subdirectory(audio_ao)
#add_subdirectory(rgn)
add_subdirectory(ircut)
add_subdirectory(uac_uvc)

include_directories(
	sys
	sensor
	vif
	isp
	scl
	rgn/st_rgn
	#uac_uvc/st_uvc
	common
	../../soc_sdk/include/isp
)

add_library(system ${DIR_APP_SRC})

target_link_libraries(system 
	sensor vif 
	isp
	scl
	#divp
	venc 
	sys 
	common
	audio_ai
	audio_ao 
	#rgn 
	ircut
	uac_uvc
)

