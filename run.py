#!/usr/bin/env python

import os

# At current stage, if we want to change between modes, we need to update source code;
# If we want to use different configuration, we need to update source code and also script configuration;

ip1 = "10.17.2.2"
ip2 = "10.17.2.2"

length = 10
directory = "/beegfs/DENG/docker"

os.system("./capture.py -a capture.conf -b {:s}:17100 {:s}:17101 {:s}:17102 {:s}:17103 {:s}:17104 {:s}:17105 -c {:f} -d {:s}".format(ip1, ip1, ip1, ip2, ip2, ip2, length, directory))
