
#option ( ADFLIB_ENABLE_TESTS "Enable tests" ON )
option ( ADFLIB_ENABLE_UNIT_TESTS "Enable units tests (require Check framework >= 0.11" ON )

add_subdirectory( regr )

if ( ADFLIB_ENABLE_UNIT_TESTS AND
    NOT ( UNIX AND MINGW ) )       # cannot build tests (ie. Check) cross-compiling
  message ( STATUS "Unit tests (in tests/unit) enabled." )
  add_subdirectory( unit )
else()
  message ( STATUS "Unit tests (in tests/unit) disabled." )
endif()


file (
  COPY ${PROJECT_SOURCE_DIR}/tests/data
  DESTINATION ${PROJECT_BINARY_DIR}/tests/
)
