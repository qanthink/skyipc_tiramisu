aux_source_directory(. DIR_APP_SRC)

include_directories(
	../queue
	../system/venc
)

add_library(ffmpeg ${DIR_APP_SRC})

#2021.8.1 全功能
#target_link_libraries(ffmpeg avformat avcodec avfilter avutil swresample swscale queue venc)

#2021.8.27 剪裁后
target_link_libraries(ffmpeg avformat avcodec avutil queue venc)

