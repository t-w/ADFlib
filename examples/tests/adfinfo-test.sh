#!/bin/sh
basedir=`dirname "$0"`
. $basedir/common.sh

ADFINFO=`get_test_cmd adfinfo`

$ADFINFO "$basedir/arccsh.adf" >$actual
compare_with "adf show metadata device" adfinfo-test_1

$ADFINFO "$basedir/arccsh.adf" 0 >$actual
compare_with "adf show metadata volume 0" adfinfo-test_2

$ADFINFO "$basedir/arccsh.adf" 0 CSH >$actual
compare_with "adf show metadata file CHS" adfinfo-test_3

$ADFINFO "$basedir/arccsh.adf" 0 c/ >$actual
compare_with "adf show metadata directory c/" adfinfo-test_4

$ADFINFO "$basedir/arccsh.adf" 0 l >$actual
compare_with "adf show metadata directory l" adfinfo-test_5

read status < $status && test "x$status" = xsuccess
