
# Add verbose output for test execution
#AM_TESTS_ENVIRONMENT = VERBOSE=1

# Define the script that needs to be generated
TESTS_ENVIRONMENT = CONFIG_SH=$(abs_top_builddir)/tests/config.sh


dist_check_SCRIPTS = \
	test_adfinfo.sh \
	test_adfinfo_hd.sh

dist_check_DATA = \
	cmd/adfinfo_hd/adfinfo_hd_test_device \
	cmd/adfinfo_hd/adfinfo_hd_test_volume_0 \
	cmd/adfinfo_hd/adfinfo_hd_test_volume_1 \
	cmd/adfinfo_hd/adfinfo_hd_test_volume_2 \
	cmd/adfinfo_hd/adfinfo_hd_test_volume_3 \
	cmd/adfinfo_hd/adfinfo_hd_test_volume_4 \
	cmd/adfinfo_hd/adfinfo_hd_test_volume_5 \
	cmd/adfinfo_hd/adfinfo_hd_test_file_vol0 \
	cmd/adfinfo_hd/adfinfo_hd_test_file_vol1 \
	cmd/adfinfo_hd/adfinfo_hd_test_file_vol2 \
	cmd/adfinfo_hd/adfinfo_hd_test_file_vol3 \
	cmd/adfinfo_hd/adfinfo_hd_test_file_vol4 \
	cmd/adfinfo_hd/adfinfo_hd_test_file_vol5 \
	cmd/adfinfo_hd/adfinfo_hd_test_dir_vol0 \
	cmd/adfinfo_hd/adfinfo_hd_test_dir_vol1 \
	cmd/adfinfo_hd/adfinfo_hd_test_dir_vol2 \
	cmd/adfinfo_hd/adfinfo_hd_test_dir_vol3 \
	cmd/adfinfo_hd/adfinfo_hd_test_dir_vol4 \
	cmd/adfinfo_hd/adfinfo_hd_test_dir_vol5 \
	cmd/adfinfo/adfinfo_test_device \
	cmd/adfinfo/adfinfo_test_volume \
	cmd/adfinfo/adfinfo_test_file \
	cmd/adfinfo/adfinfo_test_dir_c \
	cmd/adfinfo/adfinfo_test_dir_l \
	func.sh \
	gen_res_msys.sh \
	test_common.sh \
	test_runner.sh \
	results/adfinfo_hd/adfinfo_hd_test_device \
	results/adfinfo_hd/adfinfo_hd_test_volume_0 \
	results/adfinfo_hd/adfinfo_hd_test_volume_1 \
	results/adfinfo_hd/adfinfo_hd_test_volume_2 \
	results/adfinfo_hd/adfinfo_hd_test_volume_3 \
	results/adfinfo_hd/adfinfo_hd_test_volume_4 \
	results/adfinfo_hd/adfinfo_hd_test_volume_5 \
	results/adfinfo_hd/adfinfo_hd_test_file_vol0 \
	results/adfinfo_hd/adfinfo_hd_test_file_vol1 \
	results/adfinfo_hd/adfinfo_hd_test_file_vol2 \
	results/adfinfo_hd/adfinfo_hd_test_file_vol3 \
	results/adfinfo_hd/adfinfo_hd_test_file_vol4 \
	results/adfinfo_hd/adfinfo_hd_test_file_vol5 \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol0 \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol1 \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol2 \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol3 \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol4 \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol5 \
	results/adfinfo_hd/adfinfo_hd_test_device_msys \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol0_msys \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol1_msys \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol2_msys \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol3_msys \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol4_msys \
	results/adfinfo_hd/adfinfo_hd_test_dir_vol5_msys \
	results/adfinfo_hd/adfinfo_hd_test_file_vol0_msys \
	results/adfinfo_hd/adfinfo_hd_test_file_vol1_msys \
	results/adfinfo_hd/adfinfo_hd_test_file_vol2_msys \
	results/adfinfo_hd/adfinfo_hd_test_file_vol3_msys \
	results/adfinfo_hd/adfinfo_hd_test_file_vol4_msys \
	results/adfinfo_hd/adfinfo_hd_test_file_vol5_msys \
	results/adfinfo_hd/adfinfo_hd_test_volume_0_msys \
	results/adfinfo_hd/adfinfo_hd_test_volume_1_msys \
	results/adfinfo_hd/adfinfo_hd_test_volume_2_msys \
	results/adfinfo_hd/adfinfo_hd_test_volume_3_msys \
	results/adfinfo_hd/adfinfo_hd_test_volume_4_msys \
	results/adfinfo_hd/adfinfo_hd_test_volume_5_msys \
	results/adfinfo/adfinfo_test_device \
	results/adfinfo/adfinfo_test_dir_c \
	results/adfinfo/adfinfo_test_dir_l \
	results/adfinfo/adfinfo_test_file \
	results/adfinfo/adfinfo_test_volume \
	results/adfinfo/adfinfo_test_device_msys \
	results/adfinfo/adfinfo_test_dir_c_msys \
	results/adfinfo/adfinfo_test_dir_l_msys \
	results/adfinfo/adfinfo_test_file_msys \
	results/adfinfo/adfinfo_test_volume_msys

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

