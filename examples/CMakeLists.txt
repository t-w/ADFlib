
include_directories (
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_BINARY_DIR}/src
)


if ( "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC" )
  set ( ADFLIB_CUSTOM_GETOPT_SRC
    getopt.c
    getopt.h )
else()
  set ( ADFLIB_CUSTOM_GETOPT_SRC "")
endif()

add_executable ( unadf unadf.c )
target_link_libraries ( unadf adf )
set_target_properties ( unadf PROPERTIES
  #  INSTALL_RPATH "${CMAKE_INSTALL_LIBDIR}"
  INSTALL_RPATH "/usr/local/lib"
)

add_executable( adfimgcreate
  adfimgcreate.c
  pathutils.c
  pathutils.h
  ${ADFLIB_CUSTOM_GETOPT_SRC}
)
target_link_libraries ( adfimgcreate adf )
set_target_properties ( adfimgcreate PROPERTIES
  INSTALL_RPATH "/usr/local/lib"
)

add_executable ( adfformat
  adfformat.c
  ${ADFLIB_CUSTOM_GETOPT_SRC}
)
target_link_libraries ( adfformat adf )
set_target_properties ( adfformat PROPERTIES
  INSTALL_RPATH "/usr/local/lib"
)

add_executable ( adf_show_metadata
  adf_show_metadata.c
  adf_show_metadata_common.c
  adf_show_metadata_common.h
  adf_show_metadata_dir.c
  adf_show_metadata_dir.h
  adf_show_metadata_dircache.c
  adf_show_metadata_dircache.h
  adf_show_metadata_file.c
  adf_show_metadata_file.h
  adf_show_metadata_volume.c
  adf_show_metadata_volume.h
  pathutils.c
  pathutils.h
)
target_link_libraries ( adf_show_metadata adf )
set_target_properties ( adf_show_metadata PROPERTIES
  INSTALL_RPATH "/usr/local/lib"
)

add_executable ( adfbitmap adfbitmap.c )
target_link_libraries ( adfbitmap adf )
set_target_properties ( adfbitmap PROPERTIES
  INSTALL_RPATH "/usr/local/lib"
)

add_executable ( adf_salvage
  adf_salvage.c
  ${ADFLIB_CUSTOM_GETOPT_SRC}
)
target_link_libraries ( adf_salvage adf )
set_target_properties ( adf_salvage PROPERTIES
  INSTALL_RPATH "/usr/local/lib"
)

file (
  COPY ${PROJECT_SOURCE_DIR}/examples/tests
  DESTINATION ${PROJECT_BINARY_DIR}/examples/
)

if ( ADFLIB_ENABLE_TESTS )
    message ( STATUS "Enable tests in examples/" )
    add_test ( tests/test_examples_basic.sh ${BASH_PROGRAM}
               tests/test_examples_basic.sh . )
    add_test ( tests/adf-floppy-test.sh ${BASH_PROGRAM}
               tests/adf-floppy-test.sh )
    add_test ( tests/adf-show-metadata-test.sh ${BASH_PROGRAM}
               tests/adf-show-metadata-test.sh )
    add_test ( tests/unadf-test.sh ${BASH_PROGRAM}
               tests/unadf-test.sh )
endif()

install ( TARGETS
  unadf
  adfimgcreate
  adfformat
  adf_show_metadata
  adfbitmap
  adf_salvage
)
