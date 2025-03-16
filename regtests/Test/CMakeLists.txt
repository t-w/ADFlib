
#
# Test building configuration
#

find_program (BASH_PROGRAM bash)

include_directories (
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/src/generic
  ${PROJECT_BINARY_DIR}/src
)

add_executable ( fl_test fl_test.c common.c common.h )
TARGET_LINK_LIBRARIES ( fl_test adf )

add_executable ( fl_test2 fl_test2.c common.c common.h )
TARGET_LINK_LIBRARIES ( fl_test2 adf )

add_executable ( dir_test dir_test.c common.c common.h )
TARGET_LINK_LIBRARIES ( dir_test adf )

add_executable ( dir_test2 dir_test2.c common.c common.h )
TARGET_LINK_LIBRARIES ( dir_test2 adf )

add_executable ( dir_test_chdir dir_test_chdir.c )
TARGET_LINK_LIBRARIES ( dir_test_chdir adf )

add_executable ( hd_test hd_test.c common.c common.h )
TARGET_LINK_LIBRARIES ( hd_test adf )

add_executable ( hd_test2 hd_test2.c common.c common.h )
TARGET_LINK_LIBRARIES ( hd_test2 adf )

add_executable ( hd_test3 hd_test3.c common.c common.h )
TARGET_LINK_LIBRARIES ( hd_test3 adf )

add_executable ( file_test file_test.c common.c common.h )
TARGET_LINK_LIBRARIES ( file_test adf )

add_executable ( file_test2 file_test2.c common.c common.h )
TARGET_LINK_LIBRARIES ( file_test2 adf )

add_executable ( file_test3 file_test3.c common.c common.h )
TARGET_LINK_LIBRARIES ( file_test3 adf )

add_executable ( file_seek_test file_seek_test.c )
TARGET_LINK_LIBRARIES ( file_seek_test adf )

add_executable ( file_seek_test2 file_seek_test2.c )
TARGET_LINK_LIBRARIES ( file_seek_test2 adf )

add_executable ( file_seek_after_write file_seek_after_write.c )
TARGET_LINK_LIBRARIES ( file_seek_after_write adf )

add_executable ( file_read_hard_link_test file_read_hard_link_test.c )
TARGET_LINK_LIBRARIES ( file_read_hard_link_test adf )

add_executable ( del_test del_test.c common.c common.h )
TARGET_LINK_LIBRARIES ( del_test adf )

add_executable ( bootdisk bootdisk.c common.c common.h )
TARGET_LINK_LIBRARIES ( bootdisk adf )

add_executable ( cache_crash cache_crash.c )
TARGET_LINK_LIBRARIES ( cache_crash adf )

add_executable ( rename rename.c common.c common.h )
TARGET_LINK_LIBRARIES ( rename adf )

add_executable ( hardfile hardfile.c common.c common.h )
TARGET_LINK_LIBRARIES ( hardfile adf )

add_executable ( hardfile2 hardfile2.c common.c common.h )
TARGET_LINK_LIBRARIES ( hardfile2 adf )

add_executable ( rename2 rename2.c common.c common.h )
TARGET_LINK_LIBRARIES ( rename2 adf )

add_executable ( access access.c common.c common.h )
TARGET_LINK_LIBRARIES ( access adf )

add_executable ( comment comment.c common.c common.h )
TARGET_LINK_LIBRARIES ( comment adf )

add_executable ( undel undel.c common.c common.h )
TARGET_LINK_LIBRARIES ( undel adf )

add_executable ( readonly readonly.c common.c common.h )
TARGET_LINK_LIBRARIES ( readonly adf )

add_executable ( undel2 undel2.c common.c common.h )
TARGET_LINK_LIBRARIES ( undel2 adf )

add_executable ( dispsect dispsect.c common.c common.h )
TARGET_LINK_LIBRARIES ( dispsect adf )

add_executable ( progbar progbar.c common.c common.h )
TARGET_LINK_LIBRARIES ( progbar adf )

add_executable ( floppy_overfilling_test floppy_overfilling_test.c )
TARGET_LINK_LIBRARIES ( floppy_overfilling_test adf )

add_executable ( bitmap_read_segfault bitmap_read_segfault.c log.c log.h )
TARGET_LINK_LIBRARIES ( bitmap_read_segfault adf )

add_executable ( bitmap_recreate bitmap_recreate.c log.c log.h )
TARGET_LINK_LIBRARIES ( bitmap_recreate adf )

add_executable ( test_update_datasize_ofs test_update_datasize_ofs.c )
TARGET_LINK_LIBRARIES ( test_update_datasize_ofs adf )


foreach ( a_file
    floppy.sh
    bigdev.sh
    bootdisk.sh
    bitmap_read_segfault.sh
    bitmap_recreate.sh
    cache_crash.sh
    common.sh
    del_test.sh
    dir_test2.sh
    dir_test_chdir.sh
    dir_test.sh
    file_read_hard_link_test.sh
    file_seek_test2.sh
    file_seek_test.sh
    file_test2.sh
    file_test3.sh
    file_test.sh
    floppy_overfilling_test.sh
    floppy.sh
    fl_test2.sh
    fl_test.sh
    hardfile.sh
    hd_test2.sh
    hd_test3.sh
    hd_test.sh
    rename2.sh
    rename.sh
    undel2.sh
    undel.sh )
  configure_file ( ${a_file} ${a_file} COPYONLY )
endforeach ( a_file )

file (
  COPY ${PROJECT_SOURCE_DIR}/regtests/Boot
  DESTINATION ${PROJECT_BINARY_DIR}/regtests/
)

file (
  COPY ${PROJECT_SOURCE_DIR}/regtests/Dumps
  DESTINATION ${PROJECT_BINARY_DIR}/regtests/
)

file (
  COPY ${PROJECT_SOURCE_DIR}/regtests/Files
  DESTINATION ${PROJECT_BINARY_DIR}/regtests/
)


#
# Test executing configuration
#

add_test ( floppy.sh ${BASH_PROGRAM} floppy.sh )
add_test ( bigdev.sh ${BASH_PROGRAM} bigdev.sh )
add_test ( test_update_datasize_ofs test_update_datasize_ofs )
