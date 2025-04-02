#!/bin/bash
#
# Convert results for MSYS
#

set -e

main() {
    #local resultsOsPostfix=$(get_os_postfix)
    local resultsOsPostfix=_msys

    for testResDir in results/*
    do
	echo $testResDir
        local curDir=$(pwd)
        cd $testResDir
        rm -fv *_msys
        for testCmd in *
        do
            echo $testCmd
            #if [[ "$testCmd" =~ "*_msys" ]] ; then continue ; fi
            local testCmdMsys=${testCmd}${resultsOsPostfix}
            echo $testCmd $testCmdMsys
            unix2dos -v <"${testCmd}" >"${testCmdMsys}"
	done
	cd $curDir
    done
}

main

echo "REMEMBER TO CHANGE '/' TO '\\' IN PATHS!!!"
