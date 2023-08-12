aux_source_directory(. DIR_APP_SRC)

add_library(audio_ai ${DIR_APP_SRC})

target_link_libraries(audio_ai mi_ai)

