aux_source_directory(. DIR_APP_SRC)

add_library(ai ${DIR_APP_SRC})

target_link_libraries(ai mi_ai)

include_directories(../audio_ao/include)

