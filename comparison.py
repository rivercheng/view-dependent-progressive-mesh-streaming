#!/usr/bin/env python
import sys
import os
if len(sys.argv) < 3:
    print "Usage: " + sys.argv[0] + " <prefix> <count>"
    sys.exit(1)

prefix = sys.argv[1]
count  = int(sys.argv[2])
final_file_name = prefix + "_final_image.pgm"

for i in xrange(count):
    pgm_file_name = prefix + str(i) + ".pgm"
    os.system("./psnr "+final_file_name+" "+pgm_file_name)


