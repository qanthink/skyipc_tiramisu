##### build libfaac
include_directories("./")
include_directories("./include")
aux_source_directory(./ DIR_FAAC_SRC)
add_library(faac ${DIR_FAAC_SRC})
target_link_libraries(faac m)

