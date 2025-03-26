#!/bin/sh
basedir=$(dirname "$0")
. $basedir/common.sh

adfinfo=$(get_test_cmd adfinfo)

$adfinfo "$dumps_dir/arccsh.adf" >$actual
compare_with "adfinfo device" adfinfo-test_1

$adfinfo "$dumps_dir/arccsh.adf" 0 >$actual
compare_with "adfinfo volume 0" adfinfo-test_2

$adfinfo "$dumps_dir/arccsh.adf" 0 CSH >$actual
compare_with "adfinfo file CHS" adfinfo-test_3

$adfinfo "$dumps_dir/arccsh.adf" 0 c/ >$actual
compare_with "adfinfo directory c/" adfinfo-test_4

$adfinfo "$dumps_dir/arccsh.adf" 0 l >$actual
compare_with "adfinfo directory l" adfinfo-test_5

read status < $status && test "x$status" = xsuccess
