## Process this file with autoconf to produce configure script

## Copyright (C) 2010 University of Helsinki
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.


# autoconf requirements
AC_PREREQ([2.62])
LT_PREREQ([2.2.6])

# init
AC_INIT([hfstospell], [0.2.0], [hfst-bugs@helsinki.fi], [hfstospell], [http://hfst.sf.net])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.11 -Wall -Werror foreign check-news color-tests silent-rules])
AM_SILENT_RULES([yes])
AC_REVISION([$Revision: 1.9 $])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([ospell.cc])
AC_CONFIG_HEADERS([config.h])

# Information on package
HFSTOSPELL_NAME=hfstospell
HFSTOSPELL_MAJOR=0
HFSTOSPELL_MINOR=2
HFSTOSPELL_EXTENSION=.0_beta
HFSTOSPELL_VERSION=$HFSTOSPELL_MAJOR.$HFSTOSPELL_MINOR$HFSTOSPELL_EXTENSION
AC_SUBST(HFSTOSPELL_MAJOR)
AC_SUBST(HFSTOSPELL_MINOR)
AC_SUBST(HFSTOSPELL_VERSION)
AC_SUBST(HFSTOSPELL_NAME)

# Checks for programs
AC_PROG_CC
AC_PROG_CXX
AC_LIBTOOL_WIN32_DLL
LT_INIT
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries

# Checks for header files
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([getopt.h])

# Checks for types
AC_TYPE_SIZE_T

# Checks for structures

# Checks for compiler characteristics

# Checks for library functions
AC_FUNC_MALLOC
AC_CHECK_LIB([archive], [main])
AM_CONDITIONAL([CAN_ZIP], [test "x$ac_cv_lib_archive_main" != xno])
PKG_CHECK_MODULES([LIBXML], [libxml-2.0 >= 2.6.19])

# Checks for system services

# config files
AC_CONFIG_FILES([Makefile hfstospell.pc])

# output
AC_OUTPUT

## keep the list alphabetically sorted (vim; !sort)
cat <<EOF
-- Building $PACKAGE_STRING
 * zhfst support: $ac_cv_lib_archive_main
EOF

AS_IF([test "x$ac_cv_lib_archive_main" == xno], 
      [AC_MSG_WARN([libarchive missing, support for zhfst files disabled])])


