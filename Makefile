all: hfst-ospell

install: hfst-ospell.a hfst-ol.h ospell.h
	install -m 644 libhfst-ospell.a /usr/local/lib/
	install -m 644 hfst-ol.h ospell.h /usr/local/include/

libhfst-ospell.a: hfst-ol.o ospell.o
	ar cru $@ $^

hfst-ospell: hfst-ol.cc main.cc ospell.cc ospell.h hfst-ol.h Makefile
	g++ -Wall -Wextra -Werror -O3 -o hfst-ospell hfst-ol.cc ospell.cc main.cc

debug: hfst-ol.cc main.cc ospell.cc ospell.h hfst-ol.h Makefile
	g++ -g -Wall -Wextra -o hfst-ospell hfst-ol.cc ospell.cc main.cc

profile: hfst-ol.cc main.cc ospell.cc ospell.h hfst-ol.h Makefile
	g++ -Wall -Wextra -O2 -o hfst-ospell hfst-ol.cc ospell.cc main.cc -pg
