#!/usr/bin/python

# see edisdist.py --help for usage

import sys
from optparse import OptionParser

usage_string = "usage: %prog [options] alphabet"
parser = OptionParser(usage=usage_string)
parser.add_option("-e", "--epsilon", dest = "epsilon",
                  help = "specify symbol to use as epsilon", metavar = "EPS")
parser.add_option("-d", "--distance", type = "int", dest = "distance",
                  help = "specify edit depth", metavar = "DIST")
parser.set_defaults(epsilon = '@0@')
parser.set_defaults(distance = 1)
(options, args) = parser.parse_args()

if len(args) == 0:
    print "No alphabet specified!"
    sys.exit()
if len(args) > 1:
    print "Too many options!"
    sys.exit()

alphabet = unicode(args[0], 'utf-8')
epsilon = unicode(options.epsilon, 'utf-8')
distance = options.distance

for state in range(distance):
    for char in alphabet: # first the nonmodifying transitions
        print str(state) + "\t" + str(state) + "\t" + char.encode("utf-8") + "\t" + char.encode("utf-8") + "\t0.0"
    print str(state + 1) + "\t0.0" # all states except the first one are final

alphabet = [a for a in alphabet] + [epsilon] # now add epsilon to the alphabet for the edits

for char in alphabet:
    others = [c for c in alphabet if c != char]
    for other in others:
        for state in range(distance):
            print str(state) + "\t" + str(state + 1) + "\t" + char + "\t" + other + "\t1.0"
