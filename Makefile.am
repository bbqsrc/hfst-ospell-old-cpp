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

.PHONY: test

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
endif # EXTRA_DEMOS

if EXTRA_DEMOS

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

# tests
if CAN_TEST
TXTS=scripts/acceptor.basic.txt scripts/analyser.default.txt \
	 scripts/errmodel.basic.txt scripts/errmodel.extrachars.txt \
	 scripts/errmodel.edit1.txt
check_DATA=speller_basic.zhfst empty_descriptions.zhfst \
		   empty_titles.zhfst empty_locale.zhfst \
		   trailing_spaces.zhfst \
		   acceptor.basic.hfst errmodel.basic.hfst \
		   errmodel.extrachars.hfst bad_errormodel.zhfst \
		   speller_analyser.zhfst no_errormodel.zhfst \
		   speller_edit1.zhfst
# Actual test scripts:
TESTS=scripts/basic-zhfst.sh scripts/basic-edit1.sh \
	  scripts/empty-descriptions.sh scripts/empty-titles.sh scripts/empty-locale.sh \
	  scripts/trailing-spaces.sh scripts/bad-errormodel.sh scripts/empty-zhfst.sh \
	  scripts/analyse-spell.sh scripts/no-errormodel.sh
XFAIL_TESTS=scripts/empty-descriptions.sh scripts/empty-titles.sh \
	    scripts/empty-locale.sh scripts/empty-zhfst.sh
EXTRA_DIST=$(TXTS) $(TESTS) $(man1_MANS)\
		scripts/basic_test.xml scripts/empty_descriptions.xml \
		scripts/empty_titles.xml scripts/empty_locale.xml \
		scripts/trailing_spaces.xml scripts/no_errmodel.xml \
		scripts/test.strings

cleanup += $(check_DATA) index.xml
endif # CAN_TEST

# N.B. Do not parallel test, race condition exists
# N.B. Do not parallel test, race condition exists
empty_descriptions.zhfst: acceptor.basic.hfst errmodel.basic.hfst empty_descriptions.xml
	cp -f $(srcdir)/empty_descriptions.xml index.xml
	cp -f acceptor.basic.hfst acceptor.default.hfst
	cp -f errmodel.basic.hfst errmodel.default.hfst
	$(ZIP) $(ZIPFLAGS) $@ acceptor.default.hfst errmodel.default.hfst index.xml
	-rm -f index.xml acceptor.default.hfst errmodel.default.hfst

empty_titles.zhfst: acceptor.basic.hfst errmodel.basic.hfst empty_titles.xml
	cp -f $(srcdir)/empty_titles.xml index.xml
	cp -f acceptor.basic.hfst acceptor.default.hfst
	cp -f errmodel.basic.hfst errmodel.default.hfst
	$(ZIP) $(ZIPFLAGS) $@ acceptor.default.hfst errmodel.default.hfst index.xml
	-rm -f index.xml acceptor.default.hfst errmodel.default.hfst

empty_locale.zhfst: acceptor.basic.hfst errmodel.basic.hfst empty_locale.xml
	cp -f $(srcdir)/empty_locale.xml index.xml
	cp -f acceptor.basic.hfst acceptor.default.hfst
	cp -f errmodel.basic.hfst errmodel.default.hfst
	$(ZIP) $(ZIPFLAGS) $@ acceptor.default.hfst errmodel.default.hfst index.xml
	-rm -f index.xml acceptor.default.hfst errmodel.default.hfst

trailing_spaces.zhfst: acceptor.basic.hfst errmodel.basic.hfst trailing_spaces.xml
	cp -f $(srcdir)/trailing_spaces.xml index.xml
	cp -f acceptor.basic.hfst acceptor.default.hfst
	cp -f errmodel.basic.hfst errmodel.default.hfst
	$(ZIP) $(ZIPFLAGS) $@ acceptor.default.hfst errmodel.default.hfst index.xml
	-rm -f index.xml acceptor.default.hfst errmodel.default.hfst

speller_edit1.zhfst: acceptor.basic.hfst errmodel.edit1.hfst basic_test.xml
	cp $(srcdir)/basic_test.xml index.xml
	cp -f acceptor.basic.hfst acceptor.default.hfst
	cp -f errmodel.edit1.hfst errmodel.default.hfst
	$(ZIP) $(ZIPFLAGS) $@ acceptor.default.hfst errmodel.default.hfst index.xml
	-rm -f index.xml acceptor.default.hfst errmodel.default.hfst

speller_basic.zhfst: acceptor.basic.hfst errmodel.basic.hfst basic_test.xml
	cp $(srcdir)/basic_test.xml index.xml
	cp -f acceptor.basic.hfst acceptor.default.hfst
	cp -f errmodel.basic.hfst errmodel.default.hfst
	$(ZIP) $(ZIPFLAGS) $@ acceptor.default.hfst errmodel.default.hfst index.xml
	-rm -f index.xml acceptor.default.hfst errmodel.default.hfst

speller_analyser.zhfst: analyser.default.hfst errmodel.edit1.hfst basic_test.xml
	cp -f $(srcdir)/basic_test.xml index.xml
	cp -f analyser.default.hfst acceptor.default.hfst
	cp -f errmodel.edit1.hfst errmodel.default.hfst
	$(ZIP) $(ZIPFLAGS) $@ acceptor.default.hfst errmodel.default.hfst index.xml
	-rm -f index.xml acceptor.default.hfst errmodel.default.hfst

bad_errormodel.zhfst: acceptor.basic.hfst errmodel.extrachars.hfst basic_test.xml
	cp -f $(srcdir)/basic_test.xml index.xml
	cp -f acceptor.basic.hfst acceptor.default.hfst
	cp -f errmodel.extrachars.hfst errmodel.default.hfst
	$(ZIP) $(ZIPFLAGS) $@ acceptor.default.hfst errmodel.default.hfst index.xml
	-rm -f index.xml acceptor.default.hfst errmodel.default.hfst

no_errormodel.zhfst: acceptor.basic.hfst no_errmodel.xml
	cp -f $(srcdir)/no_errmodel.xml index.xml
	cp -f acceptor.basic.hfst acceptor.default.hfst
	$(ZIP) $(ZIPFLAGS) $@ acceptor.default.hfst index.xml
	-rm -f index.xml acceptor.default.hfst

.txt.hfst:
	hfst-txt2fst -e "@0@" $(HFST_FLAGS) $< | hfst-fst2fst $(HFST_FLAGS) -f olw -o $@

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
endif

test: test-runner
	$(srcdir)/test-runner

clean-local:
	-rm -rf $(cleanup) src/jni_wrap.cc java libhfstospell.jar
