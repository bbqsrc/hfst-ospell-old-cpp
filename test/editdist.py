#!/usr/bin/python

# see edisdist.py --help for usage

import sys
from optparse import OptionParser

usage_string = "usage: %prog [options] alphabet"
parser = OptionParser(usage=usage_string)
parser.add_option("-e", "--epsilon", dest = "epsilon",
                  help = "specify symbol to use as epsilon, default is @0@",
                  metavar = "EPS")
parser.add_option("-d", "--distance", type = "int", dest = "distance",
                  help = "specify edit depth, default is 1",
                  metavar = "DIST")
parser.add_option("-s", "--swap", action = "store_true", dest="swap",
                  help = "generate swaps (as well as insertions and deletions)")
parser.set_defaults(epsilon = '@0@')
parser.set_defaults(distance = 1)
parser.set_defaults(swap = False)
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
swapstate = distance + 1 # used for swap states
OTHER = u'@?@'

def p(string): # stupid python, or possibly stupid me
    return string.encode('utf-8')

for state in range(distance):
    for char in alphabet:

        # first the nonmodifying transitions
        print str(state) + "\t" + str(state) + "\t" + p(char) + "\t" + p(char) + "\t0.0"

        # then swaps of two characters - we make a separate state counter for this
        if options.swap:
            others = [c for c in alphabet if c != char]
            for other in others:
                print str(state) + "\t" + str(swapstate) + "\t" + p(char) + "\t" + p(other) + "\t1.0"
                print str(swapstate) + "\t" + str(state + 1) + "\t" + p(other) + "\t" + p(char) + "\t0.0"
                swapstate += 1 # make a new state

    # all states except swap states and the initial state are final
    print str(state + 1) + "\t0.0"

# now add epsilon to the alphabet for insertions and deletions
alphabet = [a for a in alphabet] + [epsilon]

for char in alphabet: # generate all insertions, deletions and substitutions
    others = [c for c in alphabet if c != char]
    for state in range(distance):
        for other in others:
            print str(state) + "\t" + str(state + 1) + "\t" + p(char) + "\t" + p(other) + "\t1.0"

# finally deletions and substitutions of OTHER

for state in range(distance):
    for char in alphabet:
        print str(state) + "\t" + str(state + 1) + "\t" + p(OTHER) + "\t" + p(char) + "\t1.0"
