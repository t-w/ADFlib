#!/bin/sh
basedir=`dirname "$0"`
. $basedir/common.sh

UNADF=`get_test_cmd unadf`

# check no args
check_no_args() {
    $( $UNADF >$actual 2>&1 || exit 0; )
    compare_with "check no args" unadf_1
}

check_help() {
    $UNADF -h >$actual
    compare_with "check help" unadf_help
}

# check invalid args
check_invalid_args() {
    $( $UNADF -v >$actual 2>&1 || exit 0; )
    compare_with "check invalid arg for -v" unadf_2

    $( $UNADF -d >$actual 2>&1 || exit 0; )
    compare_with "check invalid arg for -d" unadf_3
}

# -l (list root directory) option
check_list_root_directory_option() {
    $UNADF -l "$basedir/arccsh.adf" >$actual 2>/dev/null
    compare_with "-l (list root directory) option" unadf_4
}

# -r (list entire disk) option
check_list_entire_disk_option() {
    $UNADF -r "$basedir/arccsh.adf" >$actual 2>/dev/null
    compare_with "-r (list entire disk) option" unadf_5
}

# -s (show logical block pointer) option
check_show_logical_block_pointer_option() {
    $UNADF -ls "$basedir/arccsh.adf" >$actual 2>/dev/null
    compare_with "-s (show logical block pointer) option" unadf_6
}
# TODO -c (use dircache data) option
# TODO -m (display file comments) option
# TODO -v (mount different volume) option

# -d (extract to dir)
check_extract_to_dir() {
    $UNADF -d $tmpdir/x "$basedir/arccsh.adf" >$actual 2>/dev/null
    compare_with "-d (extract to dir)" unadf_7

# check permissions were set on extracted files
# (these tests are failing on MSYS2:
#  -> either the way of checking or permissions are not valid)
    if [ "x${host_type}" != 'xMINGW32' -a \
         "x${host_type}" != 'xMINGW64' ]
    then
	[ -x $tmpdir/x/CSH ] || { echo "Invalid x/CSH permissions" ;
				  echo failed >$status ; }
	[ -x $tmpdir/x/c/LZX ] || { echo "Invalid x/c/LZX permissions" ;
				    echo failed >$status ; }
    fi
}

# -d (extract to dir) with specific files, not all their original case
check_extract_to_dir_with_specific_files() {
    $UNADF -d $tmpdir/x "$basedir/arccsh.adf" csh s/startup-sequence \
        devs/system-configuration >$actual 2>/dev/null
    compare_with "-d (extract to dir) with specific files, not all their original case" \
        unadf_8
}
# TODO check permissions were set (bug: currently they aren't)

# -p (extract to pipe) option
check_extract_to_pipe_option() {
    $UNADF -p "$basedir/arccsh.adf" s/startup-sequence >$actual 2>/dev/null
    compare_with " -p (extract to pipe) option" unadf_9
}

# -w (mangle win32 filenames) option
check_mangle_win32_filenames_option() {
    $UNADF -d $tmpdir/x -w "$basedir/win32-names.adf" >$actual 2>/dev/null
    compare_with "-w (mangle win32 filenames) option" unadf_10
}

# confirm the mangling (-w) only occurs on extraction
check_confirm_the_mangling() {
    $UNADF -r -w "$basedir/win32-names.adf" >$actual 2>/dev/null
    compare_with "confirm the mangling (-w) only occurs on extraction" unadf_11
}


check_no_args
check_help
check_invalid_args
check_list_root_directory_option
check_list_entire_disk_option
check_show_logical_block_pointer_option
check_extract_to_dir
check_extract_to_dir_with_specific_files
check_extract_to_pipe_option
check_mangle_win32_filenames_option
check_confirm_the_mangling

read status < $status && test "x$status" = xsuccess
echo "status: $status"
