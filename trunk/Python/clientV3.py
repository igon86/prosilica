#!/usr/bin/env python
#This code is a client for recording "movies" broadcast by a camera server.  This code is mostly just a wrapping for gstreamer.


import sys, os, getopt

import pygtk, gtk, gobject
import pygst
pygst.require("0.10") #Probably don't need this
import gst


def usage():
    print "-h, --help     Displays this message"
    print "-n, --no-gui   Disables graphical interface"
    print "-c, --config   Path and file to load configuration from.  Default: /'./pcameraSettings.ini/'"


class GTK_Main:
    def __init__(self):

        #Default defines
        self.guiOn = True
        self.ini_file = 'pcameraSettings.ini'
        self.movieFileName = 'defaultMovie.ogm'
        self.recordingFlag = False
        
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
            self.button = gtk.Button("Start Recording: " + self.movieFileName)
            self.button.connect("clicked", self.start_stop)
            vbox.add(self.button)

            self.textBoxFileName = gtk.Entry()
            self.textBoxFileName.connect("activate",self.changeFileName)
            vbox.add(self.textBoxFileName)
            
            window.show_all()
            
        #End window setup
        self.createPipeline()
        
        if not self.guiOn:
            self.player.set_state(gst.STATE_PLAYING)

    def createPipeline(self):
        self.player = gst.Pipeline("player")
        colorspace = gst.element_factory_make("ffmpegcolorspace","colorspace")

        caps = gst.Caps("video/x-raw-yuv,framerate=50/1")
        #videorate0 = gst.element_factory_make("videorate","videorate0")
        filter = gst.element_factory_make("capsfilter","filter0")
        filter.set_property("caps",caps)
        
        self.filesink = gst.element_factory_make("filesink","filesink0")
        self.filesink.set_property("location",self.movieFileName)
		
        jpegdec0 = gst.element_factory_make("jpegdec","jpegdec0")

        theora = gst.element_factory_make("theoraenc","theoraenc0")
        oggmux = gst.element_factory_make("oggmux","oggmux0")

        udpsrc = gst.element_factory_make("udpsrc","udpsrc")
        udpsrc.set_property("port",self.UDPmulticastPort)

        udpsrc.set_property("multicast-group", self.UDPmulticastIP)
                
        self.player.add(udpsrc,jpegdec0,colorspace,theora,oggmux,self.filesink)
        gst.element_link_many(udpsrc, jpegdec0,colorspace,theora,oggmux,self.filesink)

    #Button definitions
    def start_stop(self, w):
        if self.recordingFlag == True:
            self.textBoxFileName.set_editable(True)
            self.button.set_label("Start Recording: " + self.movieFileName)
            self.player.set_state(gst.STATE_NULL)
            self.recordingFlag = False
        else:
            self.textBoxFileName.set_editable(False)
            self.button.set_label("Stop Recording: " + self.movieFileName)
            self.player.set_state(gst.STATE_PLAYING)
            self.recordingFlag = True
            
    def changeFileName(self,w):
        tempName = self.textBoxFileName.get_text()
        if tempName != self.movieFileName:
            
            self.player.set_state(gst.STATE_NULL)
            self.player = None
            self.createPipeline()
            self.movieFileName = tempName
            self.filesink.set_property("location",self.movieFileName)
            self.button.set_label("Start Recording: " + self.movieFileName)

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
