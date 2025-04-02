

# Add verbose output for test execution
AM_TESTS_ENVIRONMENT = VERBOSE=1

# Define the script that needs to be generated
TESTS_ENVIRONMENT = CONFIG_SH=$(abs_top_builddir)/tests/config.sh


dist_check_SCRIPTS = \
	test_adfinfo_hd.sh

dist_check_DATA = \
	cmd/adfinfo_hd/adfinfo_hd_test_device \
	func.sh \
	gen_res_msys.sh \
	test_common.sh \
	test_runner.sh \
	results/adfinfo_hd/adfinfo_hd_test_device \
	results/adfinfo_hd/adfinfo_hd_test_device_msys

#check_SCRIPTS = $(dist_check_SCRIPTS)
check_DATA = $(dist_check_DATA)

TESTS = $(dist_check_SCRIPTS)
#TESTS = test_adfinfo_hd.sh

# Pre-test validation
check-local:
	@echo "Running pre-test validation 2..."
	@test -f $(abs_top_builddir)/tests/config.sh && \
            echo "config.sh is present" || \
            echo "ERROR: config.sh is missing!"
