
# Add verbose output for test execution
#AM_TESTS_ENVIRONMENT = VERBOSE=1

# Define the script that needs to be generated
TESTS_ENVIRONMENT = CONFIG_SH=$(abs_top_builddir)/tests/config.sh


dist_check_SCRIPTS = \
	test_adfbitmap.sh \
	test_adfinfo.sh \
	test_adfinfo_hd.sh \
	test_adf_floppy.sh \
	test_adfsalvage.sh

dist_check_DATA = \
	func.sh \
	gen_res_msys.sh \
	test_common.sh \
	test_runner.sh \
	cmd/adfbitmap/1_show_invalid_bitmap \
	cmd/adfbitmap/2_rebuild_bitmap \
	cmd/adfbitmap/3_show_valid_bitmap \
	cmd/adf_floppy/floppy_dev_create \
	cmd/adf_floppy/floppy_dev_info \
	cmd/adf_floppy/floppy_ffs_dev_format \
	cmd/adf_floppy/floppy_ffs_dev_info \
	cmd/adf_floppy/floppy_ffs_vol_info \
	cmd/adf_floppy/floppy_ofs_dev_format \
	cmd/adf_floppy/floppy_ofs_dev_info \
	cmd/adf_floppy/floppy_ofs_vol_info \
	cmd/adfinfo/adfinfo_test_device \
	cmd/adfinfo/adfinfo_test_dir_c \
	cmd/adfinfo/adfinfo_test_dir_l \
	cmd/adfinfo/adfinfo_test_file \
	cmd/adfinfo/adfinfo_test_volume \
	cmd/adfinfo_hd/adfinfo_hd_test_device \
	cmd/adfinfo_hd/adfinfo_hd_test_dir_vol0 \
	cmd/adfinfo_hd/adfinfo_hd_test_dir_vol1 \
	cmd/adfinfo_hd/adfinfo_hd_test_dir_vol2 \
	cmd/adfinfo_hd/adfinfo_hd_test_dir_vol3 \
	cmd/adfinfo_hd/adfinfo_hd_test_dir_vol4 \
	cmd/adfinfo_hd/adfinfo_hd_test_dir_vol5 \
	cmd/adfinfo_hd/adfinfo_hd_test_file_vol0 \
	cmd/adfinfo_hd/adfinfo_hd_test_file_vol1 \
	cmd/adfinfo_hd/adfinfo_hd_test_file_vol2 \
	cmd/adfinfo_hd/adfinfo_hd_test_file_vol3 \
	cmd/adfinfo_hd/adfinfo_hd_test_file_vol4 \
	cmd/adfinfo_hd/adfinfo_hd_test_file_vol5 \
	cmd/adfinfo_hd/adfinfo_hd_test_volume_0 \
	cmd/adfinfo_hd/adfinfo_hd_test_volume_1 \
	cmd/adfinfo_hd/adfinfo_hd_test_volume_2 \
	cmd/adfinfo_hd/adfinfo_hd_test_volume_3 \
	cmd/adfinfo_hd/adfinfo_hd_test_volume_4 \
	cmd/adfinfo_hd/adfinfo_hd_test_volume_5 \
	cmd/adfsalvage/01_show_deleted_entries \
	cmd/adfsalvage/02_show_bitmap \
	cmd/adfsalvage/03_rebuild_bitmap \
	cmd/adfsalvage/04_show_bitmap \
	cmd/adfsalvage/05_salvage_deleted_file \
	cmd/adfsalvage/06_show_deleted_entries \
	cmd/adfsalvage/07_salvaged_file_info \
	cmd/adfsalvage/08_show_bitmap \
	results/adfbitmap/1_show_invalid_bitmap \
	results/adfbitmap/1_show_invalid_bitmap_msys \
	results/adfbitmap/2_rebuild_bitmap \
	results/adfbitmap/2_rebuild_bitmap_msys \
	results/adfbitmap/3_show_valid_bitmap \
	results/adfbitmap/3_show_valid_bitmap_msys \
	results/adf_floppy/floppy_dev_create \
	results/adf_floppy/floppy_dev_create_msys \
	results/adf_floppy/floppy_dev_info \
	results/adf_floppy/floppy_dev_info_msys \
	results/adf_floppy/floppy_ffs_dev_format \
	results/adf_floppy/floppy_ffs_dev_format_msys \
	results/adf_floppy/floppy_ffs_dev_info \
	results/adf_floppy/floppy_ffs_dev_info_msys \
	results/adf_floppy/floppy_ffs_vol_info \
	results/adf_floppy/floppy_ffs_vol_info_msys \
	results/adf_floppy/floppy_ofs_dev_format \
	results/adf_floppy/floppy_ofs_dev_format_msys \
	results/adf_floppy/floppy_ofs_dev_info \
	results/adf_floppy/floppy_ofs_dev_info_msys \
	results/adf_floppy/floppy_ofs_vol_info \
	results/adf_floppy/floppy_ofs_vol_info_msys \
	results/adfinfo/adfinfo_test_device \
	results/adfinfo/adfinfo_test_device_msys \
	results/adfinfo/adfinfo_test_dir_c \
	results/adfinfo/adfinfo_test_dir_c_msys \
	results/adfinfo/adfinfo_test_dir_l \
	results/adfinfo/adfinfo_test_dir_l_msys \
	results/adfinfo/adfinfo_test_file \
	results/adfinfo/adfinfo_test_file_msys \
	results/adfinfo/adfinfo_test_volume \
	results/adfinfo/adfinfo_test_volume_msys \
	results/adfinfo_hd/adfinfo_hd_test_device \
	results/adfinfo_hd/adfinfo_hd_test_device_msys \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol0 \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol0_msys \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol1 \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol1_msys \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol2 \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol2_msys \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol3 \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol3_msys \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol4 \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol4_msys \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol5 \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol5_msys \
	results/adfinfo_hd/adfinfo_hd_test_file_vol0 \
	results/adfinfo_hd/adfinfo_hd_test_file_vol0_msys \
	results/adfinfo_hd/adfinfo_hd_test_file_vol1 \
	results/adfinfo_hd/adfinfo_hd_test_file_vol1_msys \
	results/adfinfo_hd/adfinfo_hd_test_file_vol2 \
	results/adfinfo_hd/adfinfo_hd_test_file_vol2_msys \
	results/adfinfo_hd/adfinfo_hd_test_file_vol3 \
	results/adfinfo_hd/adfinfo_hd_test_file_vol3_msys \
	results/adfinfo_hd/adfinfo_hd_test_file_vol4 \
	results/adfinfo_hd/adfinfo_hd_test_file_vol4_msys \
	results/adfinfo_hd/adfinfo_hd_test_file_vol5 \
	results/adfinfo_hd/adfinfo_hd_test_file_vol5_msys \
	results/adfinfo_hd/adfinfo_hd_test_volume_0 \
	results/adfinfo_hd/adfinfo_hd_test_volume_0_msys \
	results/adfinfo_hd/adfinfo_hd_test_volume_1 \
	results/adfinfo_hd/adfinfo_hd_test_volume_1_msys \
	results/adfinfo_hd/adfinfo_hd_test_volume_2 \
	results/adfinfo_hd/adfinfo_hd_test_volume_2_msys \
	results/adfinfo_hd/adfinfo_hd_test_volume_3 \
	results/adfinfo_hd/adfinfo_hd_test_volume_3_msys \
	results/adfinfo_hd/adfinfo_hd_test_volume_4 \
	results/adfinfo_hd/adfinfo_hd_test_volume_4_msys \
	results/adfinfo_hd/adfinfo_hd_test_volume_5 \
	results/adfinfo_hd/adfinfo_hd_test_volume_5_msys \
	results/adfsalvage/01_show_deleted_entries \
	results/adfsalvage/01_show_deleted_entries_msys \
	results/adfsalvage/02_show_bitmap \
	results/adfsalvage/02_show_bitmap_msys \
	results/adfsalvage/03_rebuild_bitmap \
	results/adfsalvage/03_rebuild_bitmap_msys \
	results/adfsalvage/04_show_bitmap \
	results/adfsalvage/04_show_bitmap_msys \
	results/adfsalvage/05_salvage_deleted_file \
	results/adfsalvage/05_salvage_deleted_file_msys \
	results/adfsalvage/06_show_deleted_entries \
	results/adfsalvage/06_show_deleted_entries_msys \
	results/adfsalvage/07_salvaged_file_info \
	results/adfsalvage/07_salvaged_file_info_msys \
	results/adfsalvage/08_show_bitmap \
	results/adfsalvage/08_show_bitmap_msys

check_SCRIPTS = $(dist_check_SCRIPTS)
check_DATA = $(dist_check_DATA)

TESTS = $(dist_check_SCRIPTS)
#TESTS = test_adfinfo_hd.sh

# Pre-test validation
# (disabled: it works, but runs _after_ running tests...)
#check-local:
#	@echo "Running pre-test validation 2..."
#	@test -f $(abs_top_builddir)/tests/config.sh && \
#            echo "config.sh is present" || \
#            echo "ERROR: config.sh is missing!"

