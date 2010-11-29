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

ifeq ($(shell uname), Darwin)
	SHAREDLIB=libhfst-ospell.0.0.0.dylib
	LINKFLAGS=-dynamiclib -Wl,-dylib_install_name,libhfst-ospell.dylib
else
	SHAREDLIB=libhfst-ospell.so.0.0.0
	LINKFLAGS=-shared -Wl,-soname,libhfst-ospell.so -Wl,-no-undefined
endif

prefix=/usr/local
libdir=$(prefix)/lib
bindir=$(prefix)/bin
includedir=$(prefix)/include
STRIPFLAG=
CXXFLAGS=-fPIC -Wall -Wextra -Werror -O3
PACKAGE_NAME=hfst-ospell
VERSION=0.1
DISTDIR=$(PACKAGE_NAME)-$(VERSION)
DIST_BZIP=$(DISTDIR).tar.bz2

all: hfst-ospell libhfst-ospell.a $(SHAREDLIB)

dist: dist-bzip2

dist-bzip2: $(DIST_BZIP)

$(DIST_BZIP): $(DISTDIR)
	tar cjvf $@ $<

$(DISTDIR): hfst-ol.cc hfst-ol.h main.cc ospell.cc ospell.h README
	mkdir -p $@
	cp -v $^ $@

install: hfst-ospell libhfst-ospell.a hfst-ol.h ospell.h $(SHAREDLIB)
	if [ ! -d $(DESTDIR)$(libdir) ] ; then \
		mkdir -p $(DESTDIR)$(libdir) ;\
	fi
	install -m 644 libhfst-ospell.a $(DESTDIR)$(libdir)
	if [ ! -d $(DESTDIR)$(includedir) ] ; then \
		mkdir -p $(DESTDIR)$(includedir) ; \
	fi
	install -m 644 hfst-ol.h ospell.h $(DESTDIR)$(includedir)
	if [ ! -d $(DESTDIR)$(bindir) ] ; then \
		mkdir -p $(DESTDIR)$(bindir) ; \
	fi
	install $(STRIPFLAG) hfst-ospell $(DESTDIR)$(bindir)
	install $(STRIPFLAG) $(SHAREDLIB) $(DESTDIR)$(libdir)
	(ldconfig || true)

libhfst-ospell.a: hfst-ol.o ospell.o
	ar cru $@ $^

$(SHAREDLIB): hfst-ol.o ospell.o
	g++ $(CXXFLAGS) $(LINKFLAGS) -o $@ $^

hfst-ospell: hfst-ol.o main.o ospell.o
	g++ $(CXXFLAGS) -o $@ $^

clean:
	-rm -f hfst-ol.o ospell.o libhfst-ospell.a hfst-ospell $(SHAREDLIB)
