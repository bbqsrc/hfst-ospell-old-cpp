all: hfst-ospell

hfst-ospell: hfst-ol.cc main.cc ospell.cc ospell.h hfst-ol.h Makefile
	g++ -Wall -o hfst-ospell hfst-ol.cc ospell.cc main.cc

debug: hfst-ol.cc main.cc ospell.cc ospell.h hfst-ol.h Makefile
	g++ -g -Wall -o hfst-ospell hfst-ol.cc ospell.cc main.cc