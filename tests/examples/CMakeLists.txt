
file (
  COPY ${PROJECT_SOURCE_DIR}/tests/examples
  DESTINATION ${PROJECT_BINARY_DIR}/tests/
)

message( STATUS "Enable tests in tests/examples/" )
add_test( test_examples_basic.sh ${BASH_PROGRAM}
          test_examples_basic.sh ../../examples )
add_test( adf-floppy-test.sh ${BASH_PROGRAM}
          adf-floppy-test.sh )
add_test( adfinfo-test.sh ${BASH_PROGRAM}
          adfinfo-test.sh )
add_test( unadf-test.sh ${BASH_PROGRAM}
          unadf-test.sh )
