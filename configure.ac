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

# init
AC_INIT([hfst-ospell], [0.1.1], [hfst-bugs@helsinki.fi], [hfst-ospell], [http://hfst.sf.net])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign check-news color-tests silent-rules 1.11])
AM_SILENT_RULES([yes])
AC_REVISION([$Revision: 1.9 $])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([ospell.cc])
AC_CONFIG_HEADERS([config.h])

# Information on package
HFSTOSPELL_NAME=hfst-ospell
HFSTOSPELL_MAJOR=0
HFSTOSPELL_MINOR=1
HFSTOSPELL_EXTENSION=.1_beta
HFSTOSPELL_VERSION=$HFSTOSPELL_MAJOR.$HFSTOSPELL_MINOR$HFSTOSPELL_EXTENSION
AC_SUBST(HFSTOSPELL_MAJOR)
AC_SUBST(HFSTOSPELL_MINOR)
AC_SUBST(HFSTOSPELL_VERSION)
AC_SUBST(HFSTOSPELL_NAME)

# Checks for programs
AC_PROG_CC
AC_PROG_CXX
m4_ifdef([LT_INIT], [LT_INIT], [AC_PROG_LIBTOOL])
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries

# Checks for header files
AC_HEADER_STDBOOL

# Checks for types
AC_TYPE_SIZE_T

# Checks for structures

# Checks for compiler characteristics

# Checks for library functions
AC_FUNC_MALLOC

# Checks for system services

# config files
AC_CONFIG_FILES([Makefile hfstospell.pc])

# output
AC_OUTPUT

## keep the list alphabetically sorted (vim; !sort)
cat <<EOF
-- Building $PACKAGE_STRING
EOF

