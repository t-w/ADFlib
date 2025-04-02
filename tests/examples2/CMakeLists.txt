
file (
  COPY ${PROJECT_SOURCE_DIR}/tests/examples2
  DESTINATION ${PROJECT_BINARY_DIR}/tests/
)

message( STATUS "Enable tests in tests/examples2/" )
add_test( test_adfinfo_hd.sh ${BASH_PROGRAM}
          test_adfinfo_hd.sh )
