# $Id: configure.in,v 1.1.1.1 2007/10/28 08:36:51 rdmp1c Exp $

dnl Process this file with autoconf to produce a configure script

AC_INIT([wc],[0.1])
AC_CONFIG_SRCDIR(equivalent.cpp)
AM_INIT_AUTOMAKE
AC_PROG_CXX
AC_LANG([C++])
AC_PROG_CC
AC_PROG_INSTALL
#AC_ARG_PROGRAM

# We need ranlib to make the library, and we need to define make
AC_PROG_RANLIB
AC_PROG_MAKE_SET

# Check for GTL
AC_CHECK_HEADERS(GTL/GTL.h)

AC_CONFIG_FILES([Makefile graph/Makefile ])
AC_OUTPUT

