#!/bin/bash
#
# Generate list of files for Makefile.am
#

set -e


echo "############################# dist_check_DATA (TO UPDATE ABOVE !!!!)" > Makefile.am_update

find cmd/ -type f | sort \
    | sed -e 's/^/\t/' -e 's/$/\ \\/' \
    >> Makefile.am_update

find results/ -type f | sort \
    | sed -e 's/^/\t/' -e 's/$/\ \\/' \
    >> Makefile.am_update
