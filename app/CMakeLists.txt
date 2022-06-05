#设置头文件搜索目录
include_directories(../soc_sdk/include)
include_directories(../soc_sdk/include/drivers/cam)
include_directories(../soc_sdk/include/drivers/sensorif)
include_directories(../thirdparty/avtp/include)
include_directories(../thirdparty/libfaac/include)
include_directories(../thirdparty/libfaad/include)
include_directories(../thirdparty/openssl/include)
include_directories(../thirdparty/live555/include/BasicUsageEnvironment)
include_directories(../thirdparty/live555/include/groupsock)
include_directories(../thirdparty/live555/include/liveMedia)
include_directories(../thirdparty/live555/include/UsageEnvironment)
include_directories(../thirdparty/libffmpeg/include)

#设置库搜索路径
link_directories(../soc_sdk/dynamic)
link_directories(../thirdparty/avtp/lib)
link_directories(../thirdparty/live555/lib)
link_directories(../thirdparty/openssl/lib)
link_directories(../thirdparty/libffmpeg/lib)

#增加cmake 工程子目录
add_subdirectory(aac)
add_subdirectory(aad)
add_subdirectory(live555rtsp)
add_subdirectory(system)
add_subdirectory(testing)
#add_subdirectory(wifi)
add_subdirectory(ffmpeg)
add_subdirectory(queue)
add_subdirectory(audio_player)
add_subdirectory(spipanel)
add_subdirectory(ethernet)
add_subdirectory(sky_avtp)

include_directories(
	system/sensor
	system/vif
	system/vpe
	system/divp
	system/venc
	system/sys
	system/audio_ai
	system/audio_ao
	system/rgn
	system/rgn/st_rgn
	system/ircut
	avtp
	aac
	aad
	live555rtsp
	testing
	wifi
	wifi/include
	ffmpeg
	queue
	audio_player
	spipanel
	ethernet
	sky_avtp
)

link_directories(
	system/sensor
	system/vif
	system/vpe
	system/divp
	system/venc
	system/sys
	system/rgn
	system/audio_ai
	system/audio_ao
	system/ircut
	avtp
	live555rtsp
	testing
	#wifi
	wifi/lib/${ToolType}
	../thirdparty/live555/lib
	ffmpeg
	queue
	audio_player
	spipanel
	ethernet
	sky_avtp
)

#增加需要编译的源文件
aux_source_directory(. DIR_SRC)

#编译出可执行文件
add_executable(skyipc ${DIR_SRC}) # 生成可执行文件

#设置 target 需要链接的库
target_link_libraries(skyipc 
	testing 
	sensor vif vpe venc sys rgn ircut 
	ai ao aac aad 
	audio_player
	#wifi
	live555rtsp 
	ffmpeg
	spipanel ethernet
	sky_avtp
	audio_player
	pthread dl
)

#strip 剪裁
add_custom_target(stripTask
	COMMAND echo ================ make strip task ================
	COMMAND ls -l -h sky*
	COMMAND ${CMAKE_STRIP} -s skyipc
	#COMMAND ${CMAKE_STRIP} -s skynvr
	COMMAND ls -l -h sky*
)

