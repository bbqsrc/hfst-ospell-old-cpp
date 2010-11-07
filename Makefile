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

all: hfst-ospell libhfst-ospell.a $(SHAREDLIB)

install: hfst-ospell libhfst-ospell.a hfst-ol.h ospell.h $(SHAREDLIB)
	install -m 644 libhfst-ospell.a $(DESTDIR)$(libdir)
	install -m 644 hfst-ol.h ospell.h $(DESTDIR)$(includedir)
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
