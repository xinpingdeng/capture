#!/usr/bin/env python

import os

# At current stage, if we want to change between modes, we need to update source code;
# If we want to use different configuration, we need to update source code and also script configuration;

ip = "10.17.2.2"
length = 10
directory = "/beegfs/DENG/docker"

os.system("./capture.py -a capture.conf -b {:s} -c 17100 17101 17102 17103 17104 17105 -d {:f} -e {:s}".format(ip, length, directory))
