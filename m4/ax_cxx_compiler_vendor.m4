# ===========================================================================
#  http://www.gnu.org/software/autoconf-archive/ax_cxx_compiler_vendor.html
# ===========================================================================
#
# OBSOLETE MACRO
#
#   Please use AC_LANG_PUSH([C++]) AX_COMPILER_VENDOR AC_LANG_POP([C++])
#
# SYNOPSIS
#
#   AX_CXX_COMPILER_VENDOR(VENDOR-NAME, [NICKNAME])
#
# DESCRIPTION
#
#   Set VENDOR-NAME to the lower-case name of the compiler vendor or
#   `unknown' if the compiler's vendor is unknown. `compaq' means the CXX
#   compiler as available on Tru64/OSF1/Digital Unix on Alpha machines. If
#   NICKNAME is provided, set it to the compiler's usual name (eg. `g++',
#   `cxx', `aCC', etc.).
#
# LICENSE
#
#   Copyright (c) 2008 Ludovic Courtes <ludo@chbouib.org>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 9

AU_ALIAS([AC_CXX_COMPILER_VENDOR], [AX_CXX_COMPILER_VENDOR])
AC_DEFUN([AX_CXX_COMPILER_VENDOR],
  [dnl
   AC_OBSOLETE($[$0],[[;please use AX_COMPILER_VENDOR]])
   AC_MSG_WARN([[AX_CXX_COMPILER_VENDOR is obsolete; please use AX_COMPILER_VENDOR]])

   AC_REQUIRE([AC_PROG_CXX])

   AC_LANG_PUSH([C++])
   AX_COMPILER_VENDOR()
   AC_LANG_POP([C++])

   # be compatible with old ABI
   AS_IF([test X$ax_cv_cxx_compiler = "Xdec"],
         [$ax_cv_cxx_compiler=compact])
  ]
)dnl
