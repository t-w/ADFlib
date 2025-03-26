#!/bin/sh
basedir=`dirname "$0"`
. $basedir/common.sh

adfimgcreate=`get_test_cmd adfimgcreate`
adfformat=`get_test_cmd adfformat`
adfinfo=`get_test_cmd adfinfo`

$adfimgcreate -t dd $tmpdir/testflopdd1.adf >$actual
compare_with "adfimgcreate -t dd" adf-floppy-test_1

$adfformat -t 1 -l TestFlopDD1 -v $tmpdir/testflopdd1.adf >$actual 2>/dev/null
compare_with "adf format dd" adf-floppy-test_2

$adfinfo $tmpdir/testflopdd1.adf >$actual
compare_with "adfinfo dd floppy dev" adf-floppy-test_3

$adfinfo $tmpdir/testflopdd1.adf 0 | grep -v \
  -e Created: -e 'Last access:' -e checkSum: -e calculated -e days: \
  -e mins: -e ticks: -e coDays: -e coMins -e coTicks >$actual
compare_with "adfinfo dd floppy volume" adf-floppy-test_4

read status < $status && test "x$status" = xsuccess
