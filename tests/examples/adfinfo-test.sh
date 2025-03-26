#!/bin/sh
basedir=`dirname "$0"`
. $basedir/common.sh

ADFINFO=`get_test_cmd adfinfo`

$ADFINFO "$basedir/arccsh.adf" >$actual
compare_with "adfinfo device" adfinfo-test_1

$ADFINFO "$basedir/arccsh.adf" 0 >$actual
compare_with "adfinfo volume 0" adfinfo-test_2

$ADFINFO "$basedir/arccsh.adf" 0 CSH >$actual
compare_with "adfinfo file CHS" adfinfo-test_3

$ADFINFO "$basedir/arccsh.adf" 0 c/ >$actual
compare_with "adfinfo directory c/" adfinfo-test_4

$ADFINFO "$basedir/arccsh.adf" 0 l >$actual
compare_with "adfinfo directory l" adfinfo-test_5

read status < $status && test "x$status" = xsuccess
