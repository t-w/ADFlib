#!/bin/bash
#
# A simple test of command-line utilities.
#
# It tries to execute them with some parameters
# and shows the outputs (it does no further checks),
# fails if the execution is not possible.
#

basedir=`dirname $0`

set -e

#if [ $# -ne 1 ]
#then
#    echo 2>&1 "Usage: $0 <bin install path>"
#    exit 1
#fi
#PATH=`pwd`/$1:$PATH

#if [ $# -ne 1 ]
if [ $# -eq 1 ]
then
#    ADF_EXAMPLES_BIN="."
#else
    ADF_EXAMPLES_BIN="$1"
fi

. ${basedir}/config.sh

PATH=$ADF_EXAMPLES_BIN:$PATH
TEST_ADF=${DUMPS_DIR}/arccsh.adf

if [ ! -f "${TEST_ADF}" ]
then
    echo "Test data not available ( ${TEST_ADF} )."
    exit 1
fi

#[ -n "$srcdir" ] && TEST_ADF="$srcdir/arccsh.adf"

run_cmd()
{
    echo "============================================================="
    #stdbuf -oL
    if [ ${OSTYPE} == "msys" ]
    then
	EXEC=${1}.exe
    else
	EXEC=${1}
    fi
    shift
    echo "executing:  ${EXEC} $@"
    #${EXAMPLES_PATH}/${EXEC} $@
    ${EXEC} $@
}

CMDS[0]="unadf -r $TEST_ADF"
CMDS[1]="adfinfo $TEST_ADF"
CMDS[2]="adfinfo $TEST_ADF 0"
CMDS[2]="adfinfo $TEST_ADF 0 CSH"
CMDS[3]="adfinfo $TEST_ADF 0 c/"
CMDS[4]="adfinfo $TEST_ADF 0 l"
CMDS[5]="adfimgcreate -t dd testflopdd1.adf"
CMDS[6]="adfformat -t 1 -l TestFlopDD1 testflopdd1.adf"
CMDS[7]="adfinfo testflopdd1.adf"
CMDS[8]="rm -v testflopdd1.adf"
CMDS[9]="adfbitmap show $TEST_ADF"

for CMD in "${CMDS[@]}"
do
    run_cmd ${CMD}
done
