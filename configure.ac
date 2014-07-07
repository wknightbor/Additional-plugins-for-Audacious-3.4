#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([audacious-audacious-alt-plugins], [0.0], [andrey@delfa.net])
AC_CONFIG_SRCDIR([config.h.in])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])
AC_DEFINE_UNQUOTED([PACKAGE], "$PACKAGE_NAME", [Name of package])
AC_DEFINE_UNQUOTED([VERSION], "$PACKAGE_VERSION", [Version number of package])

LT_INIT()
PKG_CHECK_MODULES([DEPS], [glib-2.0 audacious >= 3.4],
[AC_MSG_NOTICE([glib-2.0 and audacious 3.4 are found])],
[AC_MSG_ERROR([*** glib-2.0 or audacious 3.4 not found - please install it first ***])]
)
have_maclib=no
PKG_CHECK_MODULES([MACLIB], [maclib >= 4.11.0],
	[AC_MSG_NOTICE([maclib 4.11 are found])
	have_maclib=yes],
[AC_MSG_NOTICE([*** maclib 4.11 not found - please install it first if you need to compile libape.so ***])]
)

AM_CONDITIONAL([CLIBAPE], [test x$have_maclib = xyes])

CPPFLAGS="$CPPFLAGS"
# -include config.h"
LIBS="$LIBS"

# Checks for programs.
AC_PROG_CC_C99
AC_PROG_CXX

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h glib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC

AC_CONFIG_FILES(Makefile src/Makefile src/autocue/Makefile src/libape/Makefile)
#AC_CONFIG_SUBDIRS([src])
AC_OUTPUT()
