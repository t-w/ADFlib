
# set DLL symbols export (if building a DLL)
if ( ADFLIB_BUILD_DLL )
    message ( STATUS "Exporting DLL symbols." )
    add_compile_definitions ( _EXPORTING )
endif ( ADFLIB_BUILD_DLL )

if ( ADFLIB_ENABLE_NATIVE_DEV )
# Useful info (note that CYGWIN went from WIN32 to UNIX...):
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/tutorials/How-To-Write-Platform-Checks
    if ( WIN32 OR CYGWIN OR MINGW OR MSYS OR CMAKE_C_PLATFORM_ID STREQUAL "MinGW")
        set ( ADFLIB_NATIVE_DEV_DIR win32 )
        set ( ADFLIB_NATIVE_DEV_SRC
              win32/adf_dev_driver_nativ.c
              win32/nt4_dev.c
              win32/nt4_dev.h )
    else()
        if ( UNIX AND CMAKE_SYSTEM_NAME STREQUAL "Linux" )
	    set ( ADFLIB_NATIVE_DEV_DIR linux )
            set ( ADFLIB_NATIVE_DEV_SRC linux/adf_dev_driver_nativ.c )
        else()
            set ( ADFLIB_NATIVE_DEV_DIR generic )
            set ( ADFLIB_NATIVE_DEV_SRC generic/adf_dev_driver_nativ.c )
        endif()
    endif()
else()
    set ( ADFLIB_NATIVE_DEV_DIR generic )
    set ( ADFLIB_NATIVE_DEV_SRC generic/adf_dev_driver_nativ.c )
endif()
message ( STATUS "Native device directory: ${ADFLIB_NATIVE_DEV_DIR}" )
message ( STATUS "Native device sources:   ${ADFLIB_NATIVE_DEV_SRC}" )

include_directories (
  ${PROJECT_SOURCE_DIR}/src
  ${ADFLIB_NATIVE_DEV_DIR}
  ${PROJECT_BINARY_DIR}/src
)

if ( CMAKE_BUILD_TYPE MATCHES DEBUG )
    message ( STATUS "Debug mode - building with debug_utils")
    set ( ADFLIB_DEBUG_MODULES adf_debug.c adf_debug.h )
endif ( CMAKE_BUILD_TYPE MATCHES DEBUG )


add_library ( adf
  adflib.c
  adflib.h
  adf_bitm.c
  adf_bitm.h
  adf_blk.h
  adf_blk_hd.h
  adf_byteorder.h
  adf_cache.c
  adf_cache.h
  adf_dev.c
  adf_dev_driver.h
  adf_dev_driver_dump.c
  adf_dev_driver_dump.h
  adf_dev_driver_ramdisk.c
  adf_dev_driver_ramdisk.h
  adf_dev_drivers.c
  adf_dev_drivers.h
  adf_dev_flop.c
  adf_dev_flop.h
  adf_dev.h
  adf_dev_hd.c
  adf_dev_hd.h
  adf_dev_hdfile.c
  adf_dev_hdfile.h
  adf_dev_type.c
  adf_dev_type.h
  adf_dir.c
  adf_dir.h
  adf_env.c
  adf_env.h
  adf_err.h
  adf_file_block.c
  adf_file_block.h
  adf_file.c
  adf_file.h
  adf_file_util.c
  adf_file_util.h
  adf_limits.h
  adf_link.c
  adf_link.h
  adf_prefix.h
  adf_raw.c
  adf_raw.h
  adf_salv.c
  adf_salv.h
  adf_str.c
  adf_str.h
  adf_types.h
  adf_util.c
  adf_util.h
  adf_vector.c
  adf_vector.h
  adf_version.h
  adf_vol.c
  adf_vol.h
  adf_dev_driver_nativ.h
  ${ADFLIB_NATIVE_DEV_SRC}
  ${ADFLIB_DEBUG_MODULES}
)

set_target_properties ( adf PROPERTIES
    #PUBLIC_HEADER "adflib.h"
    PUBLIC_HEADER "adflib.h;adf_bitm.h;adf_blk.h;adf_blk_hd.h;adf_cache.h;adf_dev_driver_dump.h;adf_dev_driver_nativ.h;adf_dev_driver_ramdisk.h;adf_dev_flop.h;adf_dev.h;adf_dev_hd.h;adf_dev_hdfile.h;adf_dev_type.h;adf_dir.h;adf_env.h;adf_err.h;adf_file_block.h;adf_file.h;adf_file_util.h;adf_limits.h;adf_prefix.h;adf_raw.h;adf_salv.h;adf_str.h;adf_types.h;adf_vector.h;adf_version.h;adf_vol.h"
    PRIVATE_HEADER "adf_byteorder.h;adf_debug.h;adf_link.h;adf_util.h"
    VERSION ${CMAKE_PROJECT_VERSION}
#    SOVERSION ${PROJECT_VERSION_MAJOR}
    SOVERSION 1
)

target_include_directories ( adf
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

#target_link_libraries ( adf ${SOME_LIBRARIES} )

install ( TARGETS adf
  LIBRARY
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  PUBLIC_HEADER
    DESTINATION "${CMAKE_INSTALL_PREFIX}/include/adf"
  PRIVATE_HEADER
#    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/adf"
    DESTINATION FALSE
)


# Cmake debugging - list all defined variables
# (from: https://stackoverflow.com/questions/9298278/cmake-print-out-all-accessible-variables-in-a-script )
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
