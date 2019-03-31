#!/usr/bin/env bash
# 
# bootstrap's purpose is to prepare the source code for building running the 
# autotools and generate any source needed. Once complete we can create a 
# distribution tarball by simply calling
#
#       ./configure
#       make dist 
#
# @version 1.0
#

#
# Run gengetopt to generate the cmdline.c and cmdline.h files
#
GENGETOPT=$(which gengetopt)
if [[ ! -e $GENGETOPT ]]
then 
    echo "gengetopt required to prepare source"
    exit 1
fi
$GENGETOPT --unamed-opts -a cmdline_info -F cmdline --output-dir=src/          \
    -i option_parser.ggo --set-version=$(tr -d '\n' < VERSION)

#
# run autotools
#
autoreconf -vfi

