aux_source_directory(. DIR_APP_SRC)

include_directories(
	../system/audio_ao
	../ffmpeg
)

#add_library(audio_player SHARED ${DIR_APP_SRC})
add_library(audio_player STATIC ${DIR_APP_SRC})

target_link_libraries(audio_player)

