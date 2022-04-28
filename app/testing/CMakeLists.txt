#aux_source_directory(. DIR_APP_SRC)

add_library(testing ${DIR_APP_SRC} testing.cpp)

include_directories(
	../system/venc
	../system/audio_ai
	../system/audio_ao
	../system/rgn
	../system/rgn/st_rgn
	../aac
	../aad
	../avtp
	../wifi
	../wifi/include
	../ffmpeg
	../queue
	../live555rtsp
	../spipanel
	../ethernet
	../sky_avtp
)

target_link_libraries(testing aac aad ai ao)

