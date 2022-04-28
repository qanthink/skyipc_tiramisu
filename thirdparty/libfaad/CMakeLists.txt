##### build libfaad
include_directories("./")
include_directories("./include")
aux_source_directory(./ DIR_FAAD_SRC)
add_library(faad ${DIR_FAAD_SRC})
target_link_libraries(faad m)

