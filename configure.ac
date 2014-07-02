#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([audacious-autocue-plugin], [0.0], [andrey@delfa.net])
AM_INIT_AUTOMAKE([audacious-autocue-plugin], [0.0])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_DEFINE_UNQUOTED([PACKAGE], "$PACKAGE_NAME", [Name of package])
AC_DEFINE_UNQUOTED([VERSION], "$PACKAGE_VERSION", [Version number of package])
LT_INIT
PKG_CHECK_MODULES([DEPS], [glib-2.0 audacious])
CPPFLAGS="$CPPFLAGS"
# -include config.h"
LIBS="$LIBS"

# Checks for programs.
AC_PROG_CC_C99

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h glib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC

AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT()
