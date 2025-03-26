
file (
  COPY ${PROJECT_SOURCE_DIR}/tests/examples
  DESTINATION ${PROJECT_BINARY_DIR}/tests/
)

message( STATUS "Enable tests in tests/examples/" )
add_test( test_examples_basic.sh ${BASH_PROGRAM}
          test_examples_basic.sh ../../examples )
add_test( adf_floppy_test.sh ${BASH_PROGRAM}
          adf_floppy_test.sh )
add_test( adfinfo_test.sh ${BASH_PROGRAM}
          adfinfo_test.sh )
add_test( unadf_test.sh ${BASH_PROGRAM}
          unadf_test.sh )
