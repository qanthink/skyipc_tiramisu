aux_source_directory(. DIR_APP_SRC)

add_library(audio_ao ${DIR_APP_SRC})

target_link_libraries(audio_ao mi_ao)

