#!/usr/bin/env bash
#
# cleanup is a simple script to get a clean slate. All generated files are 
# deleted allowing for minimal files to be versioned. 
#
# @version 1.0
#

if [ -e Makefile ] ; then make distclean ; fi

rm -fr aclocal.m4 autom4te.cache compile config.guess config.log            \
    config.status config.sub configure depcomp install-sh libtool           \
    ltmain.sh missing Makefile Makefile.in src/Makefile src/Makefile.in     \
    src/cmdline.c src/cmdline.h src/config.h src/config.h.in src/.deps m4   \
    jansson doc sfcp*.tar.gz Debug
 
