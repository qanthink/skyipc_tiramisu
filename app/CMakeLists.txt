#设置头文件搜索目录，第三方
include_directories(../soc_sdk/include)
include_directories(../soc_sdk/include/isp)
#include_directories(../thirdparty/libfaac/include)
#include_directories(../thirdparty/libfaad/include)
include_directories(../thirdparty/openssl/include)
include_directories(../thirdparty/live555/include/BasicUsageEnvironment)
include_directories(../thirdparty/live555/include/groupsock)
include_directories(../thirdparty/live555/include/liveMedia)
include_directories(../thirdparty/live555/include/UsageEnvironment)
include_directories(../thirdparty/libffmpeg/include)

#设置库搜索路径，第三方
link_directories(../soc_sdk/dynamic)
link_directories(../thirdparty/live555/lib)
link_directories(../thirdparty/openssl/lib)
link_directories(../thirdparty/libffmpeg/lib)

#增加cmake 工程子目录
#add_subdirectory(aac)
#add_subdirectory(aad)
add_subdirectory(live555rtsp)
add_subdirectory(system)
add_subdirectory(testing)
#add_subdirectory(wifi)
#add_subdirectory(ffmpeg)
add_subdirectory(mylib)
#add_subdirectory(audio_player)
#add_subdirectory(spipanel)
#add_subdirectory(ethernet)
#add_subdirectory(sky_avtp)

#设置头文件搜索目录，本地
include_directories(
	system/sensor
	system/vif
	system/isp
	system/vpe
	system/scl
	system/divp
	system/venc
	system/sys
	system/audio_ai
	system/audio_ao
	system/rgn
	system/rgn/st_rgn
	system/ircut
	system/common
	system/uac_uvc
	system/uac_uvc/st_uvc
	system/uac_uvc/st_uac
	#aac
	#aad
	live555rtsp
	testing
	#wifi
	#wifi/include
	#ffmpeg
	mylib/myfifo
	mylib/myqueue
	#audio_player
	#spipanel
	#ethernet
	#sky_avtp
)

#设置库搜索路径，本地
link_directories(
	system
	live555rtsp
	testing
	#wifi
	ffmpeg
	mylib
	#audio_player
	#spipanel
	#ethernet
	#sky_avtp
)

#增加需要编译的源文件
aux_source_directory(. DIR_SRC)

#编译出可执行文件
#SET(PROJECT_NAME skyipc)
add_executable(${PROJECT_NAME} ${DIR_SRC}) # 生成可执行文件

#设置 target 需要链接的库
target_link_libraries(${PROJECT_NAME}
	testing
	system
	#AED_LINUX
	#aac aad 
	#audio_player
	#wifi 
	live555rtsp
	#ffmpeg 
	mylib
	#spipanel ethernet 
	#sky_avtp
	#audio_player
	pthread
)

#strip 剪裁
add_custom_target(stripTask
	COMMAND echo ================= make strip task =================
	COMMAND find . -name 'sky*' -type f | xargs ls -l -h
	COMMAND ${CMAKE_STRIP} -s ${PROJECT_NAME}
	#COMMAND ${CMAKE_STRIP} -s skynvr
	COMMAND find . -name 'sky*' -type f | xargs ls -l -h
	COMMAND echo ================= make strip end ==================
)

#install 拷贝安装
add_custom_target(installTask
	COMMAND echo ================ make install task ================
	COMMAND cp ${PROJECT_NAME} ~/Sigmastar/customer/winShare/
	#COMMAND cp ~/Downloads/tiramisuV012_202206_usbcam/sourcecode/project/image/output/miservice/config/modules/4.9.227/g_webcam.ko ~/Sigmastar/customer/winShare/
	COMMAND cp ~/Downloads/tiramisuV012_8838+imx415/sourcecode/project/image/output/miservice/config/modules/4.9.227/g_webcam.ko ~/Sigmastar/customer/winShare/
	COMMAND echo ================ make install end =================
)

