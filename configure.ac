AC_INIT([sse-popcount],[0.1])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([speed.cpp])
AC_CONFIG_MACRO_DIR([m4])

dnl the foreign flag prevents Automake to check GNU coding standards
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_PROG_AR

AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11([noext])
AX_CHECK_X86_FEATURES


dnl Enable ansi console by default
dnl Could disable it by --disable-ansi-console
dnl this would export HAVE_ANSI_CONSOLE=1 or 0
dnl depending on user provided option
AC_ARG_ENABLE([ansi-console],
  [AS_HELP_STRING([--disable-ansi-console],
    [disable support for ansi color seqences])],
  [],
  [enable_ansi_console=yes])
AS_IF([test "X$enable_ansi_console" != "Xno"],
  [ANSI_CONSOLE=1],
  [ANSI_CONSOLE=0])
AC_DEFINE_UNQUOTED([HAVE_ANSI_CONSOLE], 
  [$ANSI_CONSOLE],
  [defined if want to support ansi color console])


AC_CONFIG_FILES([Makefile])
AC_CONFIG_HEADERS([config.h])

AC_OUTPUT
