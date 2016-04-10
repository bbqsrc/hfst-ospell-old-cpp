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

#SUBDIRS= test

# to silence:
# libtoolize: Consider adding `-I m4' to ACLOCAL_AMFLAGS in Makefile.am.
ACLOCAL_AMFLAGS=-I m4
AM_CXXFLAGS = -Wall -Wextra

AM_V_SWIG    = $(AM_V_SWIG_@AM_V@)
AM_V_SWIG_   = $(AM_V_SWIG_@AM_DEFAULT_V@)
AM_V_SWIG_0  = @echo "  SWIG     $@";

AM_V_JAVAC   = $(AM_V_JAVAC_@AM_V@)
AM_V_JAVAC_  = $(AM_V_JAVAC_@AM_DEFAULT_V@)
AM_V_JAVAC_0 = @echo "  JAVAC    $@";

AM_V_JAR     = $(AM_V_JAR_@AM_V@)
AM_V_JAR_    = $(AM_V_JAR_@AM_DEFAULT_V@)
AM_V_JAR_0   = @echo "  JAR      $@";

# targets
if EXTRA_DEMOS
CONFERENCE_DEMOS=hfst-ospell-norvig hfst-ospell-fsmnlp-2012 hfst-ospell-cicling\
				 hfst-ospell-survey hfst-ospell-lrec2013 hfst-ispell
endif # EXTRA_DEMOS

if HFST_OSPELL_OFFICE
MAYBE_HFST_OSPELL_OFFICE=hfst-ospell-office
endif # HFST_OSPELL_OFFICE

bin_PROGRAMS=
if HFST_OSPELL_BIN
bin_PROGRAMS+=hfst-ospell $(MAYBE_HFST_OSPELL_OFFICE) $(CONFERENCE_DEMOS)
man1_MANS=doc/hfst-ospell.1
endif

lib_LTLIBRARIES=libhfstospell.la

PKG_LIBS=
PKG_CXXFLAGS=

if WANT_ARCHIVE
PKG_LIBS+=$(LIBARCHIVE_LIBS)
PKG_CXXFLAGS+=$(LIBARCHIVE_CFLAGS)
endif

if WANT_LIBXMLPP
PKG_LIBS+=$(LIBXMLPP_LIBS)
PKG_CXXFLAGS+=$(LIBXMLPP_CFLAGS) -Wno-mismatched-tags -Wno-inconsistent-missing-override
endif

if WANT_TINYXML2
PKG_LIBS+=$(TINYXML2_LIBS)
PKG_CXXFLAGS+=$(TINYXML2_CFLAGS)
endif

# library parts
libhfstospell_la_SOURCES=src/hfst-ol.cc src/ospell.cc \
			 src/ZHfstOspeller.cc src/ZHfstOspellerXmlMetadata.cc
libhfstospell_la_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) $(PKG_CXXFLAGS)
libhfstospell_la_LDFLAGS=-no-undefined -version-info 4:0:0 \
			 $(PKG_LIBS)

if WANT_JNI_BINDINGS
libhfstospell_la_SOURCES+= src/jni_wrap.cc
noinst_DATA = libhfstospell.jar
endif

# link sample program against library here

if HFST_OSPELL_BIN

hfst_ospell_SOURCES=src/main.cc
hfst_ospell_LDADD=libhfstospell.la
hfst_ospell_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) \
					 $(PKG_CXXFLAGS)

endif

if HFST_OSPELL_OFFICE

hfst_ospell_office_SOURCES=src/office.cpp
hfst_ospell_office_LDADD=libhfstospell.la
hfst_ospell_office_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) $(PKG_CXXFLAGS)

endif # HFST_OSPELL_OFFICE

if EXTRA_DEMOS

hfst_ospell_norvig_SOURCES=demo/main-norvig.cc
hfst_ospell_norvig_LDADD=libhfstospell.la
hfst_ospell_norvig_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) \
							$(PKG_CXXFLAGS)

hfst_ospell_cicling_SOURCES=demo/main-cicling.cc
hfst_ospell_cicling_LDADD=libhfstospell.la
hfst_ospell_cicling_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) \
							 $(PKG_CXXFLAGS)

hfst_ospell_lrec2013_SOURCES=demo/main-lrec2013.cc
hfst_ospell_lrec2013_LDADD=libhfstospell.la
hfst_ospell_lrec2013_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) \
							$(PKG_CXXFLAGS)

hfst_ospell_survey_SOURCES=demo/main-survey.cc
hfst_ospell_survey_LDADD=libhfstospell.la
hfst_ospell_survey_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) \
							$(PKG_CXXFLAGS)

hfst_ospell_fsmnlp_2012_SOURCES=demo/main-fsmnlp-2012.cc
hfst_ospell_fsmnlp_2012_LDADD=libhfstospell.la
hfst_ospell_fsmnlp_2012_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) \
								 $(PKG_CXXFLAGS)

hfst_ispell_SOURCES=demo/main-ispell.cc
hfst_ispell_LDADD=libhfstospell.la
hfst_ispell_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) \
							$(PKG_CXXFLAGS)

endif # EXTRA_DEMOS

# install headers for library in hfst's includedir
include_HEADERS=src/hfst-ol.h src/ospell.h src/ol-exceptions.h \
		src/ZHfstOspeller.h src/ZHfstOspellerXmlMetadata.h

# pkgconfig
pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=hfstospell.pc

cleanup=

if CAN_DOXYGEN
doxygen:
	$(DOXYGEN)
endif

javasrcdir = $(srcdir)/java/fi/helsinki/hfst

src/jni_wrap.cc:
	$(AM_V_SWIG)mkdir -p $(javasrcdir) && \
		swig -c++ -java -package fi.helsinki.hfst \
		-outdir $(javasrcdir) \
		-o $(srcdir)/src/jni_wrap.cc \
		$(srcdir)/src/ZHfstOspeller.i

libhfstospell.jar: src/jni_wrap.cc
	$(AM_V_JAVAC)javac -Xlint:-options -target 7 -source 7 $(javasrcdir)/*.java
	$(AM_V_JAR)jar cf $@ -C java .

if WANT_TESTS
noinst_PROGRAMS=test-runner
test_runner_SOURCES=test/test.cc
test_runner_LDADD=libhfstospell.la
test_runner_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) $(PKG_CXXFLAGS)

test-local: test-runner
	$(srcdir)/test-runner
endif

clean-local:
	-rm -rf $(cleanup) src/jni_wrap.cc java libhfstospell.jar
