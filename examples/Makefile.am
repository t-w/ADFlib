bin_PROGRAMS = unadf \
	adfbitmap \
	adfimgcreate \
	adfformat \
	adfsalvage \
	adfinfo

dist_check_SCRIPTS = \
	tests/test_examples_basic.sh \
	tests/adf-floppy-test.sh \
	tests/adfinfo-test.sh \
	tests/unadf-test.sh
dist_check_DATA = \
	tests/arccsh.adf \
	tests/common.sh \
	tests/win32-names.adf \
	tests/results/adf-floppy-test_1 \
	tests/results/adf-floppy-test_2 \
	tests/results/adf-floppy-test_3 \
	tests/results/adf-floppy-test_4 \
	tests/results/adfinfo-test_1 \
	tests/results/adfinfo-test_2 \
	tests/results/adfinfo-test_3 \
	tests/results/adfinfo-test_4 \
	tests/results/adfinfo-test_5 \
	tests/results/unadf_no_args \
	tests/results/unadf_help \
	tests/results/unadf_invalid_args_1 \
	tests/results/unadf_invalid_args_2 \
	tests/results/unadf_list_root_directory_option \
	tests/results/unadf_list_entire_disk_option \
	tests/results/unadf_show_logical_block_pointer_option \
	tests/results/unadf_extract_to_dir \
	tests/results/unadf_extract_to_dir_with_specific_files \
	tests/results/unadf_extract_to_pipe_option \
	tests/results/unadf_mangle_win32_filenames_option \
	tests/results/unadf_confirm_the_mangling \
	tests/results_msys/adf-floppy-test_1 \
	tests/results_msys/adf-floppy-test_2 \
	tests/results_msys/adf-floppy-test_3 \
	tests/results_msys/adf-floppy-test_4 \
	tests/results_msys/adfinfo-test_1 \
	tests/results_msys/adfinfo-test_2 \
	tests/results_msys/adfinfo-test_3 \
	tests/results_msys/adfinfo-test_4 \
	tests/results_msys/adfinfo-test_5 \
	tests/results_msys/unadf_no_args \
	tests/results_msys/unadf_help \
	tests/results_msys/unadf_invalid_args_1 \
	tests/results_msys/unadf_invalid_args_2 \
	tests/results_msys/unadf_list_root_directory_option \
	tests/results_msys/unadf_list_entire_disk_option \
	tests/results_msys/unadf_show_logical_block_pointer_option \
	tests/results_msys/unadf_extract_to_dir \
	tests/results_msys/unadf_extract_to_dir_with_specific_files \
	tests/results_msys/unadf_extract_to_pipe_option \
	tests/results_msys/unadf_mangle_win32_filenames_option \
	tests/results_msys/unadf_confirm_the_mangling

check_SCRIPTS = $(dist_check_SCRIPTS)
check_DATA = $(dist_check_DATA)

TESTS = $(dist_check_SCRIPTS)

ADFLIBS = $(top_builddir)/src/libadf.la

unadf_SOURCES = unadf.c
unadf_LDADD = $(ADFLIBS)
unadf_DEPENDENCIES = $(top_builddir)/src/libadf.la

adfbitmap_SOURCES = adfbitmap.c
adfbitmap_LDADD = $(ADFLIBS)
adfbitmap_DEPENDENCIES = $(top_builddir)/src/libadf.la

adfimgcreate_SOURCES = adfimgcreate.c \
  pathutils.c \
  pathutils.h
adfimgcreate_LDADD = $(ADFLIBS)
adfimgcreate_DEPENDENCIES = $(top_builddir)/src/libadf.la

adfformat_SOURCES = adfformat.c
adfformat_LDADD = $(ADFLIBS)
adfformat_DEPENDENCIES = $(top_builddir)/src/libadf.la

adfsalvage_SOURCES = adfsalvage.c
adfsalvage_LDADD = $(ADFLIBS)
adfsalvage_DEPENDENCIES = $(top_builddir)/src/libadf.la

adfinfo_SOURCES = adfinfo.c \
  adfinfo_common.c \
  adfinfo_common.h \
  adfinfo_dir.c \
  adfinfo_dir.h \
  adfinfo_dircache.c \
  adfinfo_dircache.h \
  adfinfo_file.c \
  adfinfo_file.h \
  adfinfo_volume.c \
  adfinfo_volume.h \
  pathutils.c \
  pathutils.h
adfinfo_LDADD = $(ADFLIBS)
adfinfo_DEPENDENCIES = $(top_builddir)/src/libadf.la

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/src/$(NATIVE_DIR)

