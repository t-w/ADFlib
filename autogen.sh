#!/bin/sh

[ -d m4 ] || mkdir m4
autoreconf -iv -W all
