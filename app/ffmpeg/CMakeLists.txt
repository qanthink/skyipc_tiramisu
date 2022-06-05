aux_source_directory(. DIR_APP_SRC)

include_directories(

)

#add_library(ffmpeg SHARED ${DIR_APP_SRC})
add_library(ffmpeg STATIC ${DIR_APP_SRC})

#2021.8.1 全功能
#target_link_libraries(ffmpeg avformat avcodec avfilter avutil swresample swscale queue venc)

#2021.8.27 剪裁后
#target_link_libraries(ffmpeg avformat avcodec avutil queue venc)	#swresample 用于重采样

#2022.5.8 增加MP3解码器
target_link_libraries(ffmpeg avformat avcodec avutil swresample)

