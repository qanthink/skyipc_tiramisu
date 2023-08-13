aux_source_directory(. DIR_APP_SRC)

add_library(testing ${DIR_APP_SRC})

include_directories(
	../system/sensor
	../system/vif
	../system/isp
	../system/scl
	../system/venc
	../system/sys
	../system/audio_ai
	../system/audio_ao
	../system/rgn
	../system/rgn/st_rgn
	../system/common
	../system/ircut
	../aac
	../aad
	../avtp
	../ffmpeg
	../mylib/myfifo
	../mylib/myqueue
	../live555rtsp
	../spipanel
	../ethernet
	../sky_avtp
	../../soc_sdk/include/isp
)

target_link_libraries(testing 
)

