
file (
  COPY ${PROJECT_SOURCE_DIR}/tests/examples2
  DESTINATION ${PROJECT_BINARY_DIR}/tests/
)

message( STATUS "Enable tests in tests/examples2/" )

add_test( test_adfbitmap.sh ${BASH_PROGRAM}
          test_adfbitmap.sh )

add_test( test_adfinfo.sh ${BASH_PROGRAM}
          test_adfinfo.sh )

add_test( test_adfinfo_hd.sh ${BASH_PROGRAM}
          test_adfinfo_hd.sh )

add_test( test_adf_floppy.sh ${BASH_PROGRAM}
          test_adf_floppy.sh )

add_test( test_adfsalvage.sh ${BASH_PROGRAM}
          test_adfsalvage.sh )
