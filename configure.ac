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
AC_INIT([hfstospell], [0.3.1], [hfst-bugs@helsinki.fi], [hfstospell], [http://hfst.sf.net])

LT_PREREQ([2.2.6])

# init
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.11 -Wall -Werror foreign check-news color-tests silent-rules])
AM_SILENT_RULES([yes])
AC_REVISION([$Revision$])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([ospell.cc])
AC_CONFIG_HEADERS([config.h])

# Information on package
HFSTOSPELL_NAME=hfstospell
HFSTOSPELL_MAJOR=0
HFSTOSPELL_MINOR=3
HFSTOSPELL_EXTENSION=.1
HFSTOSPELL_VERSION=$HFSTOSPELL_MAJOR.$HFSTOSPELL_MINOR$HFSTOSPELL_EXTENSION
AC_SUBST(HFSTOSPELL_MAJOR)
AC_SUBST(HFSTOSPELL_MINOR)
AC_SUBST(HFSTOSPELL_VERSION)
AC_SUBST(HFSTOSPELL_NAME)

# Check for pkg-config first - the configuration won't work if it isn't available:
AC_PATH_PROG([PKGCONFIG], [pkg-config], [no])
AS_IF([test "x$PKGCONFIG" = xno], [AC_MSG_ERROR([pkg-config is required - please install])])
AC_PATH_PROG([DOXYGEN], [doxygen], [false])
AM_CONDITIONAL([CAN_DOXYGEN], [test "x$DOXYGEN" != xfalse])

# Settings
AC_ARG_ENABLE([extra_demos],
              [AS_HELP_STRING([--enable-extra-demos],
                              [build conference demos for science reproduction @<:@default=no@:>@])],
              [enable_extra_demos=$enableval], [enable_extra_demos=no])
AM_CONDITIONAL([EXTRA_DEMOS], [test x$enable_extra_demos != xno])
AC_ARG_ENABLE([zhfst],
              [AS_HELP_STRING([--enable-zhfst],
                              [support zipped complex automaton sets @<:@default=check@:>@])],
              [enable_zhfst=$enableval], [enable_zhfst=check])
AC_ARG_WITH([libxmlpp],
              [AS_HELP_STRING([--with-libxmlpp],
                              [support xml metadata for zipped automaton sets with libxml++-2.6 @<:@default=yes@:>@])],
              [with_libxmlpp=$withval], [with_libxmlpp=yes])
AC_ARG_WITH([tinyxml2],
              [AS_HELP_STRING([--with-tinyxml2],
                              [support xml metadata for zipped automaton sets with tinyxml2 @<:@default=no@:>@])],
              [with_tinyxml2=$withval], [with_tinyxml2=no])

AC_ARG_WITH([extract],
            [AS_HELP_STRING([--with-extract=TARGET],
                           [extract zhfst archives to tmpdir or mem @<:@default=mem@:>@])],
            [with_extract=$withval], [with_extract=mem])
AS_IF([test "x$with_extract" = xmem], [AC_DEFINE([ZHFST_EXTRACT_TO_MEM], [1],
                        [Define to extract zhfst archives to char buffer])],
          [AS_IF([test "x$with_extract" = xtmpdir],
                 [AC_DEFINE([ZHFST_EXTRACT_TO_TMPDIR], [1],
                            [Define to extract zhfst to tmp dir])],
          [AC_MSG_ERROR([Use with-extract to mem or tmpdir])])])

# Checks for programs
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_CC
AC_PROG_CXX
AC_LIBTOOL_WIN32_DLL
LT_INIT
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PATH_PROG([HFST_TXT2FST], [hfst-txt2fst], [false])
AC_PATH_PROG([HFST_FST2FST], [hfst-fst2fst], [false])
AC_PATH_PROG([ZIP], [zip], [false])
AM_CONDITIONAL([CAN_TEST],
               [test x$HFST_TXT2FST != xfalse -a x$HFST_FST2FST != xfalse -a x$ZIP != xfalse])

# Checks for libraries
AS_IF([test x$enable_zhfst != xno],
      [PKG_CHECK_MODULES([LIBARCHIVE], [libarchive > 3],
                  [AC_DEFINE([HAVE_LIBARCHIVE], [1], [Use archives])
                   enable_zhfst=yes],
                  [enable_zhfst=no])])
AM_CONDITIONAL([WANT_ARCHIVE], [test x$enable_zhfst != xno])
AS_IF([test x$with_libxmlpp != xno],
      [PKG_CHECK_MODULES([LIBXMLPP], [libxml++-2.6 >= 2.10.0],
                         [AC_DEFINE([HAVE_LIBXML], [1], [Use libxml++])
                          enable_xml=libxmlpp],
                         [AC_MSG_ERROR([libxml++ failed])
                          enable_xml=no])])
AM_CONDITIONAL([WANT_LIBXMLPP], [test x$enable_xml = xlibxmlpp])
AS_IF([test x$with_tinyxml2 != xno -a x$with_libxmlpp = xno],
      [PKG_CHECK_MODULES([TINYXML2], [tinyxml2 >= 1.0.8],
                         [AC_DEFINE([HAVE_TINYXML2], [1], [Use tinyxml])
                          enable_xml=tinyxml2],
                         [AC_MSG_ERROR([tinyxml missing])
                          enable_xml=no])])
AM_CONDITIONAL([WANT_TINYXML2], [test x$enable_xml = xtinyxml2])

PKG_CHECK_MODULES(ICU, [icu-uc >= 4], LIBS="$LIBS $ICU_LIBS")

# Checks for header files
AC_CHECK_HEADERS([getopt.h error.h])

# Checks for types
AC_TYPE_SIZE_T

# Checks for structures

# Checks for compiler characteristics

# Checks for library functions
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strndup error])
# Checks for system services

# Checks for highest supported C++ standard
AC_LANG(C++)
AX_CHECK_COMPILE_FLAG([-std=c++17], [CXXFLAGS="$CXXFLAGS -std=c++17"], [
 AX_CHECK_COMPILE_FLAG([-std=c++1z], [CXXFLAGS="$CXXFLAGS -std=c++1z"], [
  AX_CHECK_COMPILE_FLAG([-std=c++14], [CXXFLAGS="$CXXFLAGS -std=c++14"], [
   AX_CHECK_COMPILE_FLAG([-std=c++1y], [CXXFLAGS="$CXXFLAGS -std=c++1y"], [
    AX_CHECK_COMPILE_FLAG([-std=c++11], [CXXFLAGS="$CXXFLAGS -std=c++11"], [
     AX_CHECK_COMPILE_FLAG([-std=c++0x], [CXXFLAGS="$CXXFLAGS -std=c++0x"], [])
    ])
   ])
  ])
 ])
])

# config files
AC_CONFIG_FILES([Makefile hfstospell.pc])

# output
AC_OUTPUT

cat <<EOF
-- Building $PACKAGE_STRING
    * zhfst support: $enable_zhfst
    * extracting to: $with_extract
    * xml support: $enable_xml
    * conference demos: $enable_extra_demos
EOF
AS_IF([test x$with_libxmlpp != xno -a x$with_tinyxml2 != xno],
      [AC_MSG_WARN([You can only have one xml library (e.g., --with-tinyxml2 --without-libxmlpp)])])
