AC_PREREQ([2.63])
AC_INIT(
	[DCP],
	[1.0.6],
	[www.nsa.gov],
	[dcp],
	[https://github.com/NationalSecurityAgency/DCP])

AC_COPYRIGHT([AC_PACKAGE_NAME AC_PACKAGE_VERSION])

AC_CONFIG_HEADERS([include/config.h:include/config.h.in])
AC_PROG_CC([gcc clang icc cl])
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC_STDC
AC_SYS_POSIX_TERMIOS
AC_SYS_INTERPRETER
AC_LANG(C)

###############################################################################
#
#                            Check Libraries
#
###############################################################################

AC_CHECK_LIB(
	[z],
	[main],
	[AC_SUBST(
		[LIBZLIB],
		["-lz"])
		AC_DEFINE(
			[HAVE_LIBZLIB],
			[1],
			[Define to 1 if zlib found.])
	],
	[AC_MSG_FAILURE(
		[OpenSSL's Cryto library is required to build the application.])],
	[-lz]
)

AC_CHECK_LIB(
	[crypto],
	[main],
	[AC_SUBST(
		[LIBCRYPTO],
		["-lcrypto"])
		AC_DEFINE(
			[HAVE_LIBCRYPTO],
			[1],
			[Define to 1 if libcrypto found.])
	],
	[-lssl]
)

AC_CHECK_LIB(
	[db],
	[main],
	[AC_SUBST(
		[LIBDB],
		["-ldb"])
		AC_DEFINE(
			[HAVE_LIBDB],
			[1],
			[Define to 1 if libdb found.])
	],
	[-ldb]
)

AC_CHECK_LIB(
	[jansson],
	[main],
	[AC_SUBST(
		[LIBJANSSON],
		["-ljansson"])
		AC_DEFINE(
			[HAVE_LIBJANSSON],
			[1],
			[Define to 1 if libjansson found.])
	],
	[-ljansson]
)

###############################################################################
#
#                            Check Headers
#
###############################################################################

AC_CHECK_HEADER(
	[fcntl.h],
	[AC_DEFINE(
		[HAVE_FCNTL_H],
		[1],
		[Define to 1 if you have <fcnt.h>.])]
	,
	[AC_MSG_FAILURE(
		[<fcnt.h> not found])
	]
)

AC_CHECK_HEADER(
	[unistd.h],
	[AC_DEFINE(
		[HAVE_UNISTD_H],
		[1],
		[Define to 1 if you have <unistd.h>])
	],
	[AC_MSG_FAILURE(
		[<unistd.h> not found.])
	]
)

###############################################################################
#
#                            Check Programs
#
###############################################################################

AC_CHECK_PROG([GENGETOPT_CHECK],[gengetopt],[yes])
AS_IF(
	[test x"$GENGETOPT_CHECK" != x"yes"],
	[AC_MSG_FAILURE(
		[gengetopt is required for generating the project's command line parser. Please install it before continuing.])
	]
)

###############################################################################
#
#                            Legacy Code
#
###############################################################################
#
# 	Description:
#
# 		This section contains code from the original build system that has not
# 		yet been incorporated into the new buildsystem. The m4 macros especially
# 		still need to be reviewed, as I, jflopezfernandez, am not intimately
# 		familiar enough with m4 macros to immediately incorporate them.
#
#		It is a work-in-progress, however, and this section has been preserved
# 		for the purpose of aggregating pre-refactor code with every intention
# 		of both studying and re-incorporating it back into the system. After
# 		all, the problem with the previous build system was not that it did not
# 		work, but rather that it did not conform to the standard GNU build 
# 		process, which made it both confusing and harder to use.
#
###############################################################################

# AC_CONFIG_SRCDIR([src/dcp.c])
# AC_CONFIG_MACRO_DIR([m4])

#AC_CANONICAL_SYSTEM
#AC_ARG_PROGRAM

# Get 'install' or 'install-sh' and its variants
#AC_PROG_INSTALL
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

###############################################################################
#
#                            Check Types
#
###############################################################################

AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_TYPE_UINT8_T

###############################################################################
#
#                            Check Functions
#
###############################################################################

AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getcwd gethostname memset mkdir realpath strdup strerror strrchr strtol])

###############################################################################
#
#                          Set Output Variables
#
###############################################################################

AC_SUBST(PROGRAM_NAME, [AC_PACKAGE_TARNAME])
AC_SUBST(PROGRAM_VERSION, [AC_PACKAGE_VERSION])

###############################################################################
#
#                            Generate Output
#
###############################################################################

AC_OUTPUT([Makefile])
