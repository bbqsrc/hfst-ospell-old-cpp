#!/usr/bin/python

# see edisdist.py --help for usage


import sys
from optparse import OptionParser

usage_string = "usage: %prog [options] alphabet"

info_string = """With d for distance and S for size of alphabet plus one (for epsilon),
expected output is a transducer in ATT format with                              
* Swapless:                                                                     
** n + 1 states                                                                
** n(S^2 + S - 1) transitions                                                  
* Swapful:                                                                      
** n(S^2 - 3S + 3) + 1 states                                                  
** n(3S^2 - 5S + 3) transitions"""                                                 

parser = OptionParser(usage=usage_string, epilog=info_string)
parser.add_option("-e", "--epsilon", dest = "epsilon",
                  help = "specify symbol to use as epsilon, default is @0@",
                  metavar = "EPS")
parser.add_option("-d", "--distance", type = "int", dest = "distance",
                  help = "specify edit depth, default is 1",
                  metavar = "DIST")
parser.add_option("-s", "--swap", action = "store_true", dest="swap",
                  help = "generate swaps (as well as insertions and deletions)")
parser.add_option("-v", "--verbose", action = "store_true", dest="verbose",
                  help = "print some diagnostics to standard error")
parser.set_defaults(epsilon = '@0@')
parser.set_defaults(distance = 1)
parser.set_defaults(swap = False)
parser.set_defaults(verbose = False)
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
transitions = 0 # transition counter
chars = len(alphabet) + 1 # alphabet size
OTHER = u'@?@'

def p(string): # stupid python, or possibly stupid me
    return string.encode('utf-8')

for state in range(distance):
    for char in alphabet:

        # first the nonmodifying transitions
        print str(state) + "\t" + str(state) + "\t" + p(char) + "\t" + p(char) + "\t0.0"
	transitions += 1

        # then swaps of two characters - we make a separate state counter for this
        if options.swap:
            others = [c for c in alphabet if c != char]
            for other in others:
                print str(state) + "\t" + str(swapstate) + "\t" + p(char) + "\t" + p(other) + "\t1.0"
                print str(swapstate) + "\t" + str(state + 1) + "\t" + p(other) + "\t" + p(char) + "\t0.0"
                swapstate += 1 # make a new state
		transitions += 2

    # all states except swap states and the initial state are final
    print str(state + 1) + "\t0.0"

# now add epsilon to the alphabet for insertions and deletions
alphabet = [a for a in alphabet] + [epsilon]

for char in alphabet: # generate all insertions, deletions and substitutions
    others = [c for c in alphabet if c != char]
    for state in range(distance):
        for other in others:
            print str(state) + "\t" + str(state + 1) + "\t" + p(char) + "\t" + p(other) + "\t1.0"
	    transitions += 1

# finally deletions and substitutions of OTHER

for state in range(distance):
    for char in alphabet:
        print str(state) + "\t" + str(state + 1) + "\t" + p(OTHER) + "\t" + p(char) + "\t1.0"
	transitions += 1

if options.verbose:
    sys.stderr.write(str(swapstate) + " states and " + str(transitions) + " transitions written for\n"+
                     "distance " + str(distance) + " and alphabet size " + str(chars) +"\n")
