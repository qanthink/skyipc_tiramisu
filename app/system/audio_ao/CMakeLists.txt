aux_source_directory(. DIR_APP_SRC)

#add_library(ao SHARED ${DIR_APP_SRC})
add_library(ao STATIC ${DIR_APP_SRC})

target_link_libraries(ao mi_ao)

