#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "libconfig::config" for configuration ""
set_property(TARGET libconfig::config APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(libconfig::config PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libconfig.so"
  IMPORTED_SONAME_NOCONFIG "libconfig.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS libconfig::config )
list(APPEND _IMPORT_CHECK_FILES_FOR_libconfig::config "${_IMPORT_PREFIX}/lib/libconfig.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
