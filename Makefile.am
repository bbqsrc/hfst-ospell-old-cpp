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
if EXTRA_DEMOS
CONFERENCE_DEMOS=hfst-ospell-norvig hfst-ospell-fsmnlp-2012 hfst-ospell-cicling\
				 hfst-ospell-survey hfst-ospell-lrec2013 hfst-ispell
endif # EXTRA_DEMOS

bin_PROGRAMS=hfst-ospell $(CONFERENCE_DEMOS)
lib_LTLIBRARIES=libhfstospell.la
man1_MANS=hfst-ospell.1

PKG_LIBS=
PKG_CXXFLAGS=

if WANT_ARCHIVE
PKG_LIBS+=$(LIBARCHIVE_LIBS)
PKG_CXXFLAGS+=$(LIBARCHIVE_CFLAGS)
endif

if WANT_LIBXMLPP
PKG_LIBS+=$(LIBXMLPP_LIBS)
PKG_CXXFLAGS+=$(LIBXMLPP_CFLAGS)
endif

if WANT_TINYXML2
PKG_LIBS+=$(TINYXML2_LIBS)
PKG_CXXFLAGS+=$(TINYXML2_CFLAGS)
endif

# library parts
libhfstospell_la_SOURCES=hfst-ol.cc ospell.cc \
						 ZHfstOspeller.cc ZHfstOspellerXmlMetadata.cc
libhfstospell_la_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) $(PKG_CXXFLAGS)
libhfstospell_la_LDFLAGS=-no-undefined -version-info 3:2:1 \
						 $(PKG_LIBS)

# link sample program against library here
hfst_ospell_SOURCES=main.cc
hfst_ospell_LDADD=libhfstospell.la
hfst_ospell_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) \
					 $(PKG_CXXFLAGS)

if EXTRA_DEMOS
hfst_ospell_norvig_SOURCES=main-norvig.cc
hfst_ospell_norvig_LDADD=libhfstospell.la
hfst_ospell_norvig_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) \
							$(PKG_CXXFLAGS)

hfst_ospell_cicling_SOURCES=main-cicling.cc
hfst_ospell_cicling_LDADD=libhfstospell.la
hfst_ospell_cicling_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) \
							 $(PKG_CXXFLAGS)

hfst_ospell_lrec2013_SOURCES=main-lrec2013.cc
hfst_ospell_lrec2013_LDADD=libhfstospell.la
hfst_ospell_lrec2013_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) \
							$(PKG_CXXFLAGS)

hfst_ospell_survey_SOURCES=main-survey.cc
hfst_ospell_survey_LDADD=libhfstospell.la
hfst_ospell_survey_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) \
							$(PKG_CXXFLAGS)

hfst_ospell_fsmnlp_2012_SOURCES=main-fsmnlp-2012.cc
hfst_ospell_fsmnlp_2012_LDADD=libhfstospell.la
hfst_ospell_fsmnlp_2012_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) \
								 $(PKG_CXXFLAGS)
endif # EXTRA_DEMOS

hfst_ispell_SOURCES=main-ispell.cc
hfst_ispell_LDADD=libhfstospell.la
hfst_ispell_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) \
							$(PKG_CXXFLAGS)

# install headers for library in hfst's includedir
include_HEADERS=hfst-ol.h ospell.h ol-exceptions.h \
				ZHfstOspeller.h ZHfstOspellerXmlMetadata.h

# pkgconfig
pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=hfstospell.pc

# tests
if CAN_TEST
TXTS=acceptor.default.txt errmodel.default.txt
check_DATA=speller_basic.zhfst empty_descriptions.zhfst \
		   empty_titles.zhfst empty_locale.zhfst \
		   trailing_spaces.zhfst \
		   spl.hfstol sug.hfstol err.hfstol \
		   acceptor.default.hfst errmodel.default.hfst
# Actual test scripts:
TESTS=basic-zhfst.sh basic-legacy.sh basic-zhfst-fallback.sh \
	  empty-descriptions.sh empty-titles.sh empty-locale.sh \
	  trailing-spaces.sh
XFAIL_TESTS=empty-descriptions.sh empty-titles.sh empty-locale.sh
EXTRA_DIST=$(TXTS) basic_test.xml empty_descriptions.xml test.strings $(TESTS)

clean-local:
	-rm -rf $(check_DATA) index.xml
endif # CAN_TEST

# N.B. Do not parallel test, race condition exists
empty_descriptions.zhfst: acceptor.default.hfst errmodel.default.hfst empty_descriptions.xml
	cp -f $(srcdir)/empty_descriptions.xml index.xml
	$(ZIP) $(ZIPFLAGS) $@ acceptor.default.hfst errmodel.default.hfst index.xml

empty_titles.zhfst: acceptor.default.hfst errmodel.default.hfst empty_titles.xml
	cp -f $(srcdir)/empty_titles.xml index.xml
	$(ZIP) $(ZIPFLAGS) $@ acceptor.default.hfst errmodel.default.hfst index.xml

empty_locale.zhfst: acceptor.default.hfst errmodel.default.hfst empty_locale.xml
	cp -f $(srcdir)/empty_locale.xml index.xml
	$(ZIP) $(ZIPFLAGS) $@ acceptor.default.hfst errmodel.default.hfst index.xml

trailing_spaces.zhfst: acceptor.default.hfst errmodel.default.hfst trailing_spaces.xml
	cp -f $(srcdir)/trailing_spaces.xml index.xml
	$(ZIP) $(ZIPFLAGS) $@ acceptor.default.hfst errmodel.default.hfst index.xml

# N.B. Do not parallel test, race condition exists
speller_basic.zhfst: acceptor.default.hfst errmodel.default.hfst basic_test.xml
	cp $(srcdir)/basic_test.xml index.xml
	$(ZIP) $(ZIPFLAGS) $@ acceptor.default.hfst errmodel.default.hfst index.xml

sug.hfstol: acceptor.default.hfst
	-ln -sf $< $@

spl.hfstol: acceptor.default.hfst
	-ln -sf $< $@

err.hfstol: errmodel.default.hfst
	-ln -sf $< $@

.txt.hfst:
	hfst-txt2fst $(HFST_FLAGS) $< | hfst-fst2fst $(HFST_FLAGS) -f olw -o $@

.txt.hfstol:
	hfst-txt2fst $(HFST_FLAGS) $< | hfst-fst2fst $(HFST_FLAGS) -f olw -o $@
