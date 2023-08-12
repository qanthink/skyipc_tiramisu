add_subdirectory(sensor)
add_subdirectory(vif)
add_subdirectory(isp)
add_subdirectory(scl)
#add_subdirectory(vpe)
#add_subdirectory(divp)
add_subdirectory(venc)
add_subdirectory(sys)
add_subdirectory(common)
#add_subdirectory(audio_ai)
#add_subdirectory(audio_ao)
#add_subdirectory(rgn)
add_subdirectory(ircut)
add_subdirectory(uvc_uac)

aux_source_directory(sensor DIR_APP_SRC)
aux_source_directory(vif DIR_APP_SRC)
aux_source_directory(isp DIR_APP_SRC)
aux_source_directory(scl DIR_APP_SRC)
#aux_source_directory(vpe DIR_APP_SRC)
aux_source_directory(venc DIR_APP_SRC)
aux_source_directory(sys DIR_APP_SRC)
aux_source_directory(common DIR_APP_SRC)
#aux_source_directory(audio_ai DIR_APP_SRC)
#aux_source_directory(audio_ao DIR_APP_SRC)
#aux_source_directory(rgn DIR_APP_SRC)
#aux_source_directory(ircut DIR_APP_SRC)

include_directories(
	sys
	sensor
	isp
	scl
	#vpe
	rgn/st_rgn
	#uvc_uac/st_uvc
	common
	../../soc_sdk/include/isp
)

add_library(system ${DIR_APP_SRC})

target_link_libraries(system 
	sensor vif 
	isp
	scl
	#vpe divp
	venc 
	sys 
	common
	#audio_ai audio_ao 
	#rgn 
	ircut
	uvc_uac
)

