#!/usr/bin/python

# see editdist.py --help for usage


import sys
from optparse import OptionParser

usage_string = "usage: %prog [options] alphabet"

info_string = """
There are two ways to produce an edit distance transducer: a default one, by
giving the alphabet as a command line argument, or in a file with specialized
syntax.

For the default case, all the desired transitions are generated with weight 1.0.

The specification file should be in the following format:
* first, an (optional) list of tokens separated by newlines
  all transitions involving these tokens that are otherwise unspecified
  are generated with weight 1.0. Empty lines are ignored
* if you want to specify transitions, insert a line with the content "@@"
  (without the quotes)
* on the following lines, specified transitions with the form
  FROM <TAB> TO <TAB> WEIGHT
  where FROM is the source token, TO is the destination token and WEIGHT is
  a nonnegative floating point number specifying the weight. By default,
  if only one transition involving FROM and TO is specified, the same WEIGHT
  will be used to specify the transition TO -> FROM (assuming that both are
  listed in the list of tokens).
* if the command line option to generate swaps is set, you can also specify swap
  weights with
  FROM,TO <TAB> TO,FROM <TAB> WEIGHT
  Again, unspecified swaps will be generated automatically with weight 1.0.

with d for distance and S for size of alphabet plus one
(for epsilon), expected output is a transducer in ATT format with
* Swapless:
** d + 1 states
** d*(S^2 + S - 1) transitions
* Swapful:
** d*(S^2 - 3S + 3) + 1 states
** d*(3S^2 - 5S + 3) transitions
"""

parser = OptionParser(usage=usage_string, epilog=info_string)
parser.add_option("-e", "--epsilon", dest = "epsilon",
                  help = "specify symbol to use as epsilon, default is @0@",
                  metavar = "EPS")
parser.add_option("-d", "--distance", type = "int", dest = "distance",
                  help = "specify edit depth, default is 1",
                  metavar = "DIST")
parser.add_option("-s", "--swap", action = "store_true", dest="swap",
                  help = "generate swaps (as well as insertions and deletions)")
parser.add_option("-i", "--input", dest = "inputfile",
                  help = "optional file with special edit-distance syntax",
                  metavar = "INPUT")
parser.add_option("-v", "--verbose", action = "store_true", dest="verbose",
                  help = "print some diagnostics to standard error")
parser.set_defaults(epsilon = '@0@')
parser.set_defaults(distance = 1)
parser.set_defaults(swap = False)
parser.set_defaults(verbose = False)
(options, args) = parser.parse_args()

if options.inputfile == None and len(args) == 0:
    print "No alphabet or input file specified!"
    sys.exit()
if len(args) > 1:
    print "Too many options!"
    sys.exit()

if len(args) == 1:
    alphabet = [c for c in unicode(args[0], 'utf-8')]
else:
    alphabet = []

if options.inputfile != None:
    try:
        inputfile = open(options.inputfile)
    except IOError:
        print "Couldn't open " + options.inputfile
        sys.exit()
    while True:
        line = inputfile.readline()
        if line in ["@@\n", ""]:
            break
        if line.strip() != "":
            alphabet.append(unicode(line.strip("\n"), 'utf-8'))

epsilon = unicode(options.epsilon, 'utf-8')
OTHER = u'@?@'

def p(string): # stupid python, or possibly stupid me
    return string.encode('utf-8')

def maketrans(from_st, to_st, from_sy, to_sy, weight):
    return str(from_st) + "\t" + str(to_st) + "\t" + p(from_sy) + "\t" + p(to_sy) + "\t" + str(weight)

class Transducer:
    def __init__(self, alphabet, _other = OTHER, _epsilon = epsilon):
        self.alphabet = alphabet
        self.substitutions = {}
        self.swaps = {}
        self.other = _other
        self.epsilon = _epsilon
        self.swapstate = options.distance + 1 # used for swap states
        self.transitions = []

    def process(self, specification):
        parts = specification.split('\t')
        if len(parts) != 3:
            raise ValueError("Got specification with " + str(len(parts)) +\
                                 " parts, expected 3:\n" + specification)
        weight = float(parts[2])
        if ',' in parts[0]:
            frompair = tuple(parts[0].split(','))
            topair = tuple(parts[1].split(','))
            if not (len(frompair) == len(topair) == 2):
                raise ValueError("Got swap-specification with incorrect number "
                                 "of comma separators:\n" + specification)
            if (frompair, topair) not in self.swaps:
                self.swaps[(frompair, topair)] = weight
        else:
            if not (parts[0], parts[1]) in self.substitutions:
                self.substitutions[(parts[0], parts[1])] = weight

    def generate(self):
        if (self.other, self.epsilon) not in self.substitutions:
            self.substitutions[(self.other, self.epsilon)] = 1.0
        for symbol in self.alphabet:
            if (self.other, symbol) not in self.substitutions:
                self.substitutions[(self.other, symbol)] = 1.0
            if (symbol, symbol) not in self.substitutions:
                self.substitutions[(symbol, symbol)] = 1.0
            if (self.epsilon, symbol) not in self.substitutions:
                self.substitutions[(self.epsilon, symbol)] = 1.0
            if (symbol, self.epsilon) not in self.substitutions:
                self.substitutions[(symbol, self.epsilon)] = 1.0
            for symbol2 in self.alphabet:
                if symbol == symbol2: continue
                if ((symbol, symbol2), (symbol2, symbol)) not in self.swaps:
                    if ((symbol2, symbol), (symbol, symbol2)) in self.swaps:
                        self.swaps[((symbol, symbol2), (symbol2, symbol))] = self.swaps[((symbol2, symbol), (symbol, symbol2))]
                    else:
                        self.swaps[((symbol, symbol2), (symbol2, symbol))] = 1.0
                if (symbol, symbol2) not in self.substitutions:
                    if (symbol2, symbol) in self.substitutions:
                        self.substitutions[(symbol, symbol2)] = self.substitutions[(symbol2, symbol)]
                    else:
                        self.substitutions[(symbol, symbol2)] = 1.0

    def make_transitions(self):
        for state in range(options.distance):
            self.transitions.append(str(state + 1) + "\t0.0")
            for sub in self.substitutions:
                self.transitions.append(maketrans(state, state + 1, sub[0], sub[1], self.substitutions[sub]))
            if options.swap:
                for swap in self.swaps:
                    self.transitions.append(maketrans(state, self.swapstate, swap[0][0], swap[0][1], self.swaps[swap]))
                    self.transitions.append(maketrans(self.swapstate, state + 1, swap[1][0], swap[1][1], 0.0))
                    self.swapstate += 1

transducer = Transducer(alphabet)

if options.inputfile != None:
    while True:
        line = inputfile.readline()
        if line == "\n":
            continue
        if line == "":
            break
        transducer.process(line)

transducer.generate()
transducer.make_transitions()
for transition in transducer.transitions:
    print transition

if options.verbose:
    sys.stderr.write("\n" + str(transducer.swapstate) + " states and " + str(len(transducer.transitions)) + " transitions written for\n"+
                     "distance " + str(options.distance) + " and base alphabet size " + str(len(transducer.alphabet) + 1) +"\n")
