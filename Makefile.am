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

bin_PROGRAMS=hfst-ospell hfst-ospell-office $(CONFERENCE_DEMOS)
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
libhfstospell_la_LDFLAGS=-no-undefined -version-info 4:0:0 \
						 $(PKG_LIBS)

# link sample program against library here
hfst_ospell_SOURCES=main.cc
hfst_ospell_LDADD=libhfstospell.la
hfst_ospell_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) \
					 $(PKG_CXXFLAGS)

hfst_ospell_office_SOURCES=office.cpp
hfst_ospell_office_LDADD=libhfstospell.la
hfst_ospell_office_CXXFLAGS=$(AM_CXXFLAGS) $(CXXFLAGS) $(PKG_CXXFLAGS)

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
TXTS=acceptor.basic.txt analyser.default.txt \
	 errmodel.basic.txt errmodel.extrachars.txt errmodel.edit1.txt
check_DATA=speller_basic.zhfst empty_descriptions.zhfst \
		   empty_titles.zhfst empty_locale.zhfst \
		   trailing_spaces.zhfst \
		   acceptor.basic.hfst errmodel.basic.hfst \
		   errmodel.extrachars.hfst bad_errormodel.zhfst \
		   speller_analyser.zhfst no_errormodel.zhfst \
		   speller_edit1.zhfst
# Actual test scripts:
TESTS=basic-zhfst.sh basic-edit1.sh \
	  empty-descriptions.sh empty-titles.sh empty-locale.sh \
	  trailing-spaces.sh bad-errormodel.sh empty-zhfst.sh \
	  analyse-spell.sh no-errormodel.sh
XFAIL_TESTS=empty-descriptions.sh empty-titles.sh empty-locale.sh empty-zhfst.sh
EXTRA_DIST=$(TXTS) $(TESTS) $(man1_MANS)\
			basic_test.xml empty_descriptions.xml empty_titles.xml \
			empty_locale.xml trailing_spaces.xml no_errmodel.xml \
			test.strings

clean-local:
	-rm -rf $(check_DATA) index.xml
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
