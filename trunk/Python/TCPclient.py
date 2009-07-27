#!/usr/bin/env python

import sys, os, getopt

import pygtk, gtk, gobject
import pygst
pygst.require("0.10") #Probably don't need this
import gst

cameraUID = 32223
UDPmulticastIP = "239.255.1.1"
UDPmulticastPort = 5000

def usage():
    print "-h, --help          Displays this message"
    print "-p, --local-port    Sets the port the TCP client should connect to on"


#Default
localPort = None


#Parse command line options
try:
   opts,args = getopt.getopt(sys.argv[1:],"hp:c:",["help","local-port="])
except getopt.GetoptError:
   usage()
   sys.exit(2)

if len(opts) == 0:
    usage()
    sys.exit()

for o, a in opts:
    
    if o in ("-h", "--help"):
        usage()
	sys.exit()
    elif o in ("-p", "--local-port"):
        localPort = str(a)    
    else:
        usage()
        sys.exit()

print localPort
if localPort == None:
	print "Local port is required"
	sys.exit()
	
commandString = ''.join(["gst-launch tcpclientsrc  host=127.0.0.1  port = ",str(localPort), " ! jpegdec ! ffmpegcolorspace ! ximagesink"])

os.system(commandString)

