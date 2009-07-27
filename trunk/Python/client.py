#!/usr/bin/env python

import sys, os, getopt

import pygtk, gtk, gobject
import pygst
pygst.require("0.10") #Probably don't need this
import gst

import psnap

import pcamerasrc



def usage():
    print "-h, --help     Displays this message"
    print "-n, --no-gui   Disables graphical interface"
    print "-c, --config   Path and file to load configuration from.  Default: /'./pcameraSettings.ini/'"


class GTK_Main:
    def __init__(self):

        #Default defines
        self.guiOn = True
        self.ini_file = 'pcameraSettings.ini'
        
        try:
            opts,args = getopt.getopt(sys.argv[1:],"hnc:",["help","no-gui","config="])
        except getopt.GetoptError:
            usage()
            sys.exit(2)
            
        for o, a in opts:
            if o in ("-h", "--help"):
                usage()
                sys.exit()
            elif o in ("-n", "--no-gui"):
                self.guiOn = False
            elif o in ("-c", "--config"):
                self.ini_file = str(a)
            else:
                assert False, "unhandled option"
        #Loads settings from file
        if not self.loadSettings(self.ini_file):
             self.cameraUID = 32223
             self.UDPmulticastIP = "239.255.1.1"
             self.UDPmulticastPort = 5000
             self.guiOn = True
             print "Using built-in defaults"
        
            #Window setup
        if self.guiOn:
            self.window_name = 'Camera Client'
            
            window = gtk.Window(gtk.WINDOW_TOPLEVEL)
            window.set_title(self.window_name)
            window.set_default_size(300, -1)
            window.connect("destroy", gtk.main_quit, "WM destroy")
            vbox = gtk.VBox()
            window.add(vbox)
            self.button = gtk.Button("Start")
            self.button.connect("clicked", self.start_stop)
            vbox.add(self.button)
            
            window.show_all()
            
        #End window setup

        self.player = gst.Pipeline("player")
        colorspace = gst.element_factory_make("ffmpegcolorspace","colorspace")
        
        imgsink = gst.element_factory_make("ximagesink","imgsink")
        #imgsink.set_property("sync","false")
        
		
        jpegdec0 = gst.element_factory_make("jpegdec","jpegdec0")

        avidemux = gst.element_factory_make("avidemux","avidemux")

        udpsrc = gst.element_factory_make("udpsrc","udpsrc")
        udpsrc.set_property("port",self.UDPmulticastPort)
        udpsrc.set_property("multicast-group", self.UDPmulticastIP)
        
        self.player.add(udpsrc, jpegdec0,colorspace,imgsink)
        gst.element_link_many(udpsrc, jpegdec0,colorspace,imgsink)
        
        if not self.guiOn:
            self.player.set_state(gst.STATE_PLAYING)

    #Button definitions
    def start_stop(self, w):
        if self.button.get_label() == "Start":
            self.button.set_label("Stop")
            self.player.set_state(gst.STATE_PLAYING)
        else:
            self.player.set_state(gst.STATE_NULL)
            self.button.set_label("Start")
            

    def loadSettings(self,fileName):
        if not os.path.isfile(fileName):
            sys.stderr.write(''.join(['Could not find file: ',str(fileName),'\n']))
            return False
        try:
            configFileHandle = open(fileName,'r')
            self.cameraUID = 32223
            self.UDPmulticastIP = "239.255.1.1"
            self.UDPmulticastPort = 5000
            for line in configFileHandle:
                data = line.split('=',1)
                if len(data) == 2:
                    param = data[0].strip()
                    value = data[1].strip()
                if param == 'UID':
                    self.cameraUID = int(value)
                if param == 'UDP Multicast IP':
                    self.UDPmulticastIP = value
                if param == 'UDP Multicast Port':
                    self.UDPmulticastPort = int(value)
            return True 
        except IOError:
            sys.stderr.write(''.join(['Error opening or reading from config file: ',self.configFileName,'\n']))
            return False
        except ValueError:
            sys.stderr.write(''.join(['Syntax error in config file: ',self.configFileName,'\n']))
            sys.stderr.write(''.join(['Did not understand: ',line,'\n']))
            return False
        
GTK_Main()
gtk.gdk.threads_init()
gtk.main()
