# This is for generating error sources - give an alphabet as the argument,
# get (currently) editdist-1 without swaps. I use hfst-repeat for bigger
# distances.

import sys

epsilon = u'<>'

if len(sys.argv) < 2:
    sys.exit()
if len(sys.argv) > 2:
    epsilon = unicode(sys.argv[2], 'utf-8')
alphabet = unicode(sys.argv[1], 'utf-8')

for char in alphabet:
    print "0\t0\t" + char.encode("utf-8") + "\t" + char.encode("utf-8") + "\t1.0"
    print "1\t1\t" + char.encode("utf-8") + "\t" + char.encode("utf-8") + "\t1.0"
alphabet = [a for a in alphabet] + [epsilon]
for char in alphabet:
    temp = [c for c in alphabet if c != char]
    for c in temp:
        print "0\t1\t" + char.encode("utf-8") + "\t" + c.encode("utf-8") + "\t1.0"
print "1\t0.0"
