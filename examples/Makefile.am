bin_PROGRAMS = unadf \
	adfbitmap \
	adfimgcreate \
	adf_format \
	adf_salvage \
	adf_show_metadata

dist_check_SCRIPTS = \
	tests/test_examples_basic.sh \
	tests/adf-floppy-test.sh \
	tests/adf-show-metadata-test.sh \
	tests/unadf-test.sh
dist_check_DATA = \
	tests/arccsh.adf \
	tests/common.sh \
	tests/win32-names.adf \
	tests/results/adf-floppy-test_1 \
	tests/results/adf-floppy-test_2 \
	tests/results/adf-floppy-test_3 \
	tests/results/adf-floppy-test_4 \
	tests/results/adf-show-metadata-test_1 \
	tests/results/adf-show-metadata-test_2 \
	tests/results/adf-show-metadata-test_3 \
	tests/results/adf-show-metadata-test_4 \
	tests/results/adf-show-metadata-test_5 \
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
	tests/results_msys/adf-show-metadata-test_1 \
	tests/results_msys/adf-show-metadata-test_2 \
	tests/results_msys/adf-show-metadata-test_3 \
	tests/results_msys/adf-show-metadata-test_4 \
	tests/results_msys/adf-show-metadata-test_5 \
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

adf_format_SOURCES = adf_format.c
adf_format_LDADD = $(ADFLIBS)
adf_format_DEPENDENCIES = $(top_builddir)/src/libadf.la

adf_salvage_SOURCES = adf_salvage.c
adf_salvage_LDADD = $(ADFLIBS)
adf_salvage_DEPENDENCIES = $(top_builddir)/src/libadf.la

adf_show_metadata_SOURCES = adf_show_metadata.c \
  adf_show_metadata_common.c \
  adf_show_metadata_common.h \
  adf_show_metadata_dir.c \
  adf_show_metadata_dir.h \
  adf_show_metadata_dircache.c \
  adf_show_metadata_dircache.h \
  adf_show_metadata_file.c \
  adf_show_metadata_file.h \
  adf_show_metadata_volume.c \
  adf_show_metadata_volume.h \
  pathutils.c \
  pathutils.h
adf_show_metadata_LDADD = $(ADFLIBS)
adf_show_metadata_DEPENDENCIES = $(top_builddir)/src/libadf.la

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/src/$(NATIVE_DIR)

