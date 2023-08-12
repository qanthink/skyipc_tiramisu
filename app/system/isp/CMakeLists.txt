aux_source_directory(. DIR_APP_SRC)

include_directories(
	../common
	../../../soc_sdk/include/isp
)

add_library(isp ${DIR_APP_SRC})

target_link_libraries(isp mi_isp mi_iqserver 
	cus3a ispalgo sigma3a cam_os_wrapper fbc_decode)

