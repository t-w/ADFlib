#!/bin/bash
#
# Generate list of files for Makefile.am
#

set -e

output=Makefile.am

echo 'dist_check_DATA = \' >$output

find . -type f | grep -v -e Makefile.am -e gen_dist_check_data.sh \
    | sort \
    | sed -e 's/^/\t/' -e 's/$/\ \\/' \
    >>$output

truncate -s -2 $output
