all: hfst-ospell

hfst-ospell: hfst-ol.cc main.cc ospell.cc ospell.h hfst-ol.h Makefile
	g++ -Wall -Wextra -Werror -O2 -o hfst-ospell hfst-ol.cc ospell.cc main.cc

debug: hfst-ol.cc main.cc ospell.cc ospell.h hfst-ol.h Makefile
	g++ -g -Wall -Wextra -o hfst-ospell hfst-ol.cc ospell.cc main.cc
