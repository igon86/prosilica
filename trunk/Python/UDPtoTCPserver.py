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
    print "-p, --local-port    Sets the port your TCP client should connect to on this server machine"
    print "-c, --config        Path and file to load configuration from.  Default: ./pcameraSettings.ini"

#Default
fileName = './pcameraSettings.ini'
localPort = None


#Parse command line options
try:
   opts,args = getopt.getopt(sys.argv[1:],"hp:c:",["help","local-port=","config="])
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
    elif o in ("-c", "--config"):
        fileName = str(a)
    elif o in ("-p", "--local-port"):
        localPort = str(a)    
    else:
        usage()
        sys.exit()

print localPort
if localPort == None:
	print "Local port is required"
	sys.exit()
	
if not os.path.isfile(fileName):
    sys.stderr.write(''.join(['Could not find file: ',str(fileName),'\n']))
    sys.exit()
try:
    configFileHandle = open(fileName,'r')
    cameraUID = 32223
    UDPmulticastIP = "239.255.1.1"
    UDPmulticastPort = 5000
    for line in configFileHandle:
        data = line.split('=',1)
	if len(data) == 2:
	    param = data[0].strip()
	    value = data[1].strip()
	    if param == 'UID':
	        cameraUID = int(value)
	    if param == 'UDP Multicast IP':
	        UDPmulticastIP = value
	    if param == 'UDP Multicast Port':
	        UDPmulticastPort = int(value) 
except IOError:
    sys.stderr.write(''.join(['Error opening or reading from config file: ',self.configFileName,'\n']))
except ValueError:
    sys.stderr.write(''.join(['Syntax error in config file: ',self.configFileName,'\n']))
    sys.stderr.write(''.join(['Did not understand: ',line,'\n']))

commandString = ''.join(["gst-launch udpsrc multicast-group = ",str(UDPmulticastIP)," port = ",str(UDPmulticastPort), " caps = image/jpeg ! tcpserversink host = \'127.0.0.1\' port = ",str(localPort)])

os.system(commandString)
