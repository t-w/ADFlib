#!/bin/sh
basedir=$(dirname "$0")
. $basedir/common.sh

adfimgcreate=$(get_test_cmd adfimgcreate)
adfformat=$(get_test_cmd adfformat)
adfinfo=$(get_test_cmd adfinfo)

dumpfile=$tmpdir/testflopdd1.adf

$adfimgcreate -t dd $dumpfile >$actual
compare_with "adfimgcreate -t dd" adf_floppy_test_1

$adfformat -t 1 -l TestFlopDD1 -v $dumpfile >$actual 2>/dev/null
compare_with "adf format dd" adf_floppy_test_2

$adfinfo $dumpfile >$actual
compare_with "adfinfo dd floppy dev" adf_floppy_test_3

$adfinfo $dumpfile 0 | grep -v \
  -e Created: -e 'Last access:' -e checkSum: -e calculated -e days: \
  -e mins: -e ticks: -e coDays: -e coMins -e coTicks >$actual
compare_with "adfinfo dd floppy volume" adf_floppy_test_4

read status < $status && test "x$status" = xsuccess
