dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
#AC_INIT([dcp], m4_esyscmd([tr -d '\n' < VERSION]))
AC_INIT(
	[DCP],
	[1.0.6],
	[],
	[dcp],
	[https://github.com/NationalSecurityAgency/DCP])

AC_LANG(C)
AC_PROG_CC

AC_CHECK_HEADERS(zlib.h, [], [AC_ERROR([A working zlib is required])])
AC_SEARCH_LIBS(deflate, z, [], [AC_ERROR([A working zlib is required])])

AC_CHECK_PROG(GENGETOPT_CHECK, gengetopt, yes)
AS_IF([test x"$GENGETOPT_CHECK" != x"yes"], [AC_MSG_ERROR([Please install gengetopt before trying to build package.])])

dnl Process Makefile.in to create Makefile


# AC_MSG_NOTICE([testing...])

AC_CONFIG_HEADERS([include/config.h])
# AC_CONFIG_SRCDIR([src/dcp.c])
# AC_CONFIG_MACRO_DIR([m4])

#AC_CANONICAL_SYSTEM
#AC_ARG_PROGRAM

# Get 'install' or 'install-sh' and its variants
#meAC_PROG_INSTALL
#AC_PROG_LN_S
#AC_PROG_SED
#AC_PROG_AWK

# AM_INIT_AUTOMAKE([-Wall -Werror])

# # Checks for programs.
# AC_PROG_CC
# #AM_PROG_CC_C_O
# AC_PROG_MAKE_SET

# # LibTool
# LT_INIT
# AC_PROG_LIBTOOL


AC_CHECK_HEADERS([fcntl.h stddef.h stdint.h stdlib.h string.h unistd.h])
a=1
AC_CHECK_HEADER(jansson.h, [], [a=0])
if test $a == 0
then
	AC_MSG_ERROR([jannson library not installed.  Try 'apt-get install libjansson-dev' or whatever package manager you happen to be using...])
fi

a=1
AC_CHECK_HEADER(openssl/md5.h, [], [a=0])
if test $a == 0
then
	AC_MSG_ERROR([ssl headers not install.  Try 'apt-get install libssl-dev'])
fi

# a=1
# AC_CHECK_HEADER(db.h, [], [a=0])
# if test $a == 0
# then
# 	AC_MSG_ERROR([Berkeley DB headers not installed. Try 'apt-get install libdb-dev'])
# fi

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_TYPE_UINT8_T

# # Checks for library functions.
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getcwd gethostname memset mkdir realpath strdup strerror strrchr strtol])

AC_OUTPUT([Makefile])
