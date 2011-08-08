## Process this file with automake to produce Makefile.in

# Copyright 2010 University of Helsinki
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

# to silence: 
# libtoolize: Consider adding `-I m4' to ACLOCAL_AMFLAGS in Makefile.am.
ACLOCAL_AMFLAGS=-I m4

# targets
bin_PROGRAMS=hfst-ospell hfst-ospell-norvig
lib_LTLIBRARIES=libhfstospell.la

# library parts
libhfstospell_la_SOURCES=hfst-ol.cc ospell.cc ZHfstOspeller.cc
libhfstospell_la_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) $(LIBXML_CFLAGS)
libhfstospell_la_LDFLAGS=-no-undefined -version-info 1:0:0 $(LIBXML_LIBS)

# link sample program against library here
hfst_ospell_SOURCES=main.cc
hfst_ospell_LDADD=libhfstospell.la
hfst_ospell_norvig_SOURCES=main-norvig.cc
hfst_ospell_norvig_LDADD=libhfstospell.la

# install headers for library in hfst's includedir
include_HEADERS=hfst-ol.h ospell.h ZHfstOspeller.h

# pkgconfig
pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=hfstospell.pc

