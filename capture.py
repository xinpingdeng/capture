#!/usr/bin/env python

# tempo2 -f mypar.par -pred "sitename mjd1 mjd2 freq1 freq2 ntimecoeff nfreqcoeff seg_length"

# ./capture.py -c capture.conf -n 1 -l 3600

import os, time, threading, ConfigParser, argparse, socket, json, struct, sys

def ConfigSectionMap(section):
    dict1 = {}
    options = Config.options(section)
    for option in options:
        try:
            dict1[option] = Config.get(section, option)
            if dict1[option] == -1:
                DebugPrint("skip: %s" % option)
        except:
            print("exception on %s!" % option)
            dict1[option] = None
    return dict1

## Get center frequency from multi cast
#multicast_group = '224.1.1.1'
#server_address = ('', 5007)
#sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # Create the socket
#sock.bind(server_address) # Bind to the server address
#group = socket.inet_aton(multicast_group)
#mreq = struct.pack('4sL', group, socket.INADDR_ANY)
#sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)  # Tell the operating system to add the socket to the multicast group on all interfaces.
##sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, mreq)  
#pkt, address = sock.recvfrom(1<<16)
#data = json.loads(pkt)#['beams_direction']#['beam01']
##sock.shutdown(socket.SHUT_RDWR)
#sock.close()
#freq = float(data['sky_frequency'])

freq = 1340.5
hdr  = 0
print "The centre frequency is {:.1f}MHz".format(freq)

# Read in command line arguments
parser = argparse.ArgumentParser(description='Fold data from BMF stream')
parser.add_argument('-a', '--cfname', type=str, nargs='+',
                    help='The name of configuration file')
parser.add_argument('-b', '--ip', type=str, nargs='+',
                    help='On which ip we receive data')
parser.add_argument('-c', '--port', type=int, nargs='+',
                    help='On which port we receive data')
parser.add_argument('-d', '--length', type=float, nargs='+',
                help='Length of data receiving')
parser.add_argument('-e', '--directory', type=str, nargs='+',
                    help='In which directory we record the data and read configuration files and parameter files')

args         = parser.parse_args()
cfname       = args.cfname[0]
ip           = args.ip[0]
length       = args.length[0]
directory    = args.directory[0]
port         = args.port

# Play with configuration file
Config = ConfigParser.ConfigParser()
Config.read(cfname)

# Basic configuration
nsamp_df     = int(ConfigSectionMap("BasicConf")['nsamp_df'])
npol_samp    = int(ConfigSectionMap("BasicConf")['npol_samp'])
ndim_pol     = int(ConfigSectionMap("BasicConf")['ndim_pol'])
nchk_nic     = int(ConfigSectionMap("BasicConf")['nchk_nic'])
sleep_time   = int(ConfigSectionMap("BasicConf")['sleep_time'])
ncpu_numa    = int(ConfigSectionMap("BasicConf")['ncpu_numa'])

# Capture configuration
capture_ncpu    = int(ConfigSectionMap("CaptureConf")['ncpu'])
capture_ndf 	= int(ConfigSectionMap("CaptureConf")['ndf'])
capture_nbuf    = ConfigSectionMap("CaptureConf")['nblk']
capture_key     = ConfigSectionMap("CaptureConf")['key']
#capture_key     = format(int("0x{:s}".format(capture_key), 0) + 2 * nic, 'x')
capture_key     = format(int("0x{:s}".format(capture_key), 0), 'x')
capture_efname  = ConfigSectionMap("CaptureConf")['efname']
capture_hfname  = ConfigSectionMap("CaptureConf")['hfname']
capture_nreader = ConfigSectionMap("CaptureConf")['nreader']
capture_sod     = ConfigSectionMap("CaptureConf")['sod']
capture_rbufsz  = capture_ndf *  nchk_nic * 7168

def capture():
    print ("./paf_capture -a {:s} -b {:s} -c {:d} -d {:d} -e {:s}:{:d} -e {:s}:{:d} -e {:s}:{:d} -e {:s}:{:d} -e {:s}:{:d} -e {:s}:{:d} -f {:s} -g {:s} -i {:f} -j {:f} -k {:s}".format(capture_key, capture_sod, capture_ndf, hdr, ip, port[0], ip, port[1], ip, port[2], ip, port[3], ip, port[4], ip, port[5], capture_hfname, capture_efname, freq, length, directory))
    os.system("./paf_capture -a {:s} -b {:s} -c {:d} -d {:d} -e {:s}:{:d} -e {:s}:{:d} -e {:s}:{:d} -e {:s}:{:d} -e {:s}:{:d} -e {:s}:{:d} -f {:s} -g {:s} -i {:f} -j {:f} -k {:s}".format(capture_key, capture_sod, capture_ndf, hdr, ip, port[0], ip, port[1], ip, port[2], ip, port[3], ip, port[4], ip, port[5], capture_hfname, capture_efname, freq, length, directory))
    
def main():
    os.system("dada_db -l -p -k {:s} -b {:d} -n {:s} -r {:s}".format(capture_key, capture_rbufsz, capture_nbuf, capture_nreader))
    capture()
    os.system("dada_db -k {:s} -d".format(capture_key))
    
if __name__ == "__main__":
    main()
