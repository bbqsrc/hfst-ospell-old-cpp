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
HFSTOSPELL_EXTENSION=.1
HFSTOSPELL_VERSION=$HFSTOSPELL_MAJOR.$HFSTOSPELL_MINOR$HFSTOSPELL_EXTENSION
AC_SUBST(HFSTOSPELL_MAJOR)
AC_SUBST(HFSTOSPELL_MINOR)
AC_SUBST(HFSTOSPELL_VERSION)
AC_SUBST(HFSTOSPELL_NAME)

# Settings
AC_ARG_WITH([extract_to],
            [AS_HELP_STRING([--with-extract-to=TARGET],
                           [extract zhfst archives to tmpdir or mem @<:@default=mem@:>@])]
            [], [with_extract_to=mem])
AS_IF([test x$with_extract_to=xmem], [AC_DEFINE([ZHFST_EXTRACT_TO_MEM], [1], 
          [Define to extract zhfst archives to char buffer])],
      [AS_IF([test x$with_extract_to=tmpdir], [AC_DEFINE([ZHFST_EXTRACT_TO_TMPDIR], [1],
                 [Define to extract zhfst to tmp dir])],
                 [AC_MSG_ERROR([must define --with-extract-to=mem or =tmpdir])])])

# Checks for programs
AC_PROG_CC
AC_PROG_CXX
AC_LIBTOOL_WIN32_DLL
LT_INIT
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries
AC_CHECK_LIB([archive], [main])
PKG_CHECK_MODULES([LIBXMLPP], [libxml++-2.6 >= 2.10.0], 
                  [AC_DEFINE([HAVE_LIBXML], [1], [Use libxml++])])

# Checks for header files
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([getopt.h])
AC_CHECK_HEADERS([archive.h archive_entry.h])
AM_CONDITIONAL([CAN_ZIP], [test "x$ac_cv_lib_archive_main" != xno -a "x$ac_cv_header_archive_h" != xno])

# Checks for types
AC_TYPE_SIZE_T

# Checks for structures

# Checks for compiler characteristics

# Checks for library functions
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strndup])
# Checks for system services

# config files
AC_CONFIG_FILES([Makefile hfstospell.pc])

# output
AC_OUTPUT

cat <<EOF
-- Building $PACKAGE_STRING
 * zhfst support: $ac_cv_lib_archive_main
 * libxmlpp pkg: $pkg_cv_LIBXMLPP_LIBS
EOF

AS_IF([test "x$ac_cv_lib_archive_main" == xno -o "x$ac_cv_header_archive_h" == xno], 
      [AC_MSG_WARN([libarchive missing, support for zhfst files disabled])])


