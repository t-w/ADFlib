#!/bin/sh
basedir=$(dirname "$0")
. $basedir/common.sh

adfinfo=$(get_test_cmd adfinfo)

$adfinfo "$dumps_dir/arccsh.adf" >$actual
compare_with "adfinfo device" adfinfo_test_1

$adfinfo "$dumps_dir/arccsh.adf" 0 >$actual
compare_with "adfinfo volume 0" adfinfo_test_2

$adfinfo "$dumps_dir/arccsh.adf" 0 CSH >$actual
compare_with "adfinfo file CHS" adfinfo_test_3

$adfinfo "$dumps_dir/arccsh.adf" 0 c/ >$actual
compare_with "adfinfo directory c/" adfinfo_test_4

$adfinfo "$dumps_dir/arccsh.adf" 0 l >$actual
compare_with "adfinfo directory l" adfinfo_test_5

read status < $status && test "x$status" = xsuccess
