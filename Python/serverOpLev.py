#!/usr/bin/python
'''This file loads a config file (default: pcameraSettings.ini), takes images from the indicated Prosilica camera, broadcasts the images via UDP multi-casting, as well as fitting and recording basic parameters about the '''

#----------------------------------------------------------
#Start python imports
import sys, os, getopt

import numpy
import scipy
from scipy import ndimage
from numpy import greater
from numpy import ogrid
from scipy.io import write_array

import thread
import threading
import time
import Image

import pygtk, gtk

import gobject
import pygst
pygst.require("0.10") #Probably don't need this
import gst

import psnap

import pcamerasrc


#End python imports
#------------------------------------------------------------


#--------------------------------------------------------------
#Function for printing usage of the server

def usage():
    print "-h, --help     Displays this message"
    print "-n, --no-gui   Disables graphical interface"
    print "-c, --config  Path and file to load configuration from.  Default: /'./pcameraSettings.ini/'"

#End command line option handling
#-------------------------------------------------------------

#-------------------------------------------------------------------
#Definition of a fitting thread which will run "parallel" with the video broadcast thread
#Not that because of python's Global Interpreter Lock (GIL) what really happens is these run sequentially
#but get stopped part way through
class ImageFitThread(threading.Thread):

    def __init__(self, mainThread):
        self.callingThread = mainThread 
        self.numpyArray = mainThread.numpyArray
        self.computationOn = mainThread.computationOn
        self.HeightWidth = mainThread.HeightWidth
        self.handOffFlag = mainThread.handOffFlag
        self.exposure = mainThread.exposure
        self.autoExpose = mainThread.autoExpose
        self.source = mainThread.source
        self.text = mainThread.text
        self.jpegSnapFileName = mainThread.jpegSnapFileName
        self.takeSnapShotFlag = mainThread.takeSnapShotFlag
        self.jpegRunningSnapFlag = mainThread.jpegRunningSnapFlag
        self.jpegRunningSnapDelay = mainThread.jpegRunningSnapDelay
        self.lastSnapShotTime = None
        self.player = mainThread.player
        
        self.comXChannel = mainThread.comXChannel
        self.comYChannel = mainThread.comYChannel
        self.varXChannel = mainThread.varXChannel
        self.varYChannel = mainThread.varYChannel
        self.varXYChannel = mainThread.varXYChannel
        threading.Thread.__init__(self)
		
    def run(self):
        while self.computationOn[0]:
            if self.handOffFlag[0] == 0:
            
                #Auto exposure loop
                if (self.autoExpose[0] == True):
                    if numpy.max(self.numpyArray) < 150:
                        self.exposure[0] = int(self.exposure[0]*225/tempMax)
                        self.source.set_property("exposure",self.exposure[0])
                    elif numpy.max(self.numpyArray) == 255:
                        self.exposure[0] = int(self.exposure[0]/2)
                        self.source.set_property("exposure",self.exposure[0])


                if (self.takeSnapShotFlag[0] == True):
                    print self.jpegSnapFileName[0]
                    
                    #snapShot = Image.new('L',[self.HeightWidth[1],self.HeightWidth[0]])
                    #snapShot.putdata(list(self.numpyArray[0].flat))
                    #jpegSaveName = self.jpegSnapFileName[0]
                    #snapShot.save(jpegSaveName,"JPEG")
                    #self.takeSnapShotFlag[0] = False

                    self.player.set_state(gst.STATE_NULL)
                    scipy.io.write_array(self.jpegSnapFileName[0],self.numpyArray[0])
                    self.player.set_state(gst.STATE_PLAYING)

                    self.takeSnapShotFlag[0] = False

                    
                    

                if (self.jpegRunningSnapFlag[0] == True):
                    if (time.time() - self.jpegRunningSnapDelay[0] > self.lastSnapShotTime):
                        #snapShot = Image.new('L',[self.HeightWidth[1],self.HeightWidth[0]])
                        #snapShot.putdata(list(self.numpyArray[0].flat))
                        #jpegSaveName = ''.join([self.jpegSnapFileName[0],'.',str(time.time())])
                        #snapShot.save(jpegSaveName,"JPEG")
                        #self.lastSnapShotTime = time.time()

                        self.player.set_state(gst.STATE_NULL)
                        scipy.io.write_array(''.join([self.jpegSnapFileName[0],'.',str(time.time())]),self.numpyArray[0])
                        self.lastSnapShotTime = time.time()
                        self.player.set_state(gst.STATE_PLAYING)               

                    
                #The second input to greater is the hard coded noise floor
                #below which we ignore values for statistical calculations
                maskedArray = greater(self.numpyArray[0],25)*self.numpyArray[0]
						
                #Center of Mass, Covariance Matrix computations 
                totalLight = scipy.sum(maskedArray)
                [comX,comY] =  scipy.ndimage.measurements.center_of_mass(maskedArray)
				
                coord = ogrid[(-comX):((self.HeightWidth[0]-1.0)-comX):complex(0,self.HeightWidth[0]),
                              (-comY):((self.HeightWidth[1]-1.0)-comY):complex(0,self.HeightWidth[1])]
				
                xxcoord = coord[0]**2
                yycoord = coord[1]**2
                xycoord = coord[0]*coord[1]
				
                xxvar = scipy.sum(xxcoord*maskedArray)/totalLight
                yyvar = scipy.sum(yycoord*maskedArray)/totalLight
                xyvar = scipy.sum(xycoord*maskedArray)/totalLight

				
                stdDevX = xxvar**0.5
                stdDevY = yyvar**0.5
                if self.callingThread.overlayOn:
                    self.text.set_property("text",''.join(["Camera: ",str(self.callingThread.cameraUID),"\n X axis CoM: ",str(comY),"\n Y axis CoM: ", str(comX), "\n"]))

                else:
                    self.text.set_property("text","")
                #os.system(''.join(['ezcawrite ',self.comXChannel,' \'',str(comY),'\'']))
                #os.system(''.join(['ezcawrite ',self.comYChannel,' \'',str(comX),'\'']))
                #os.system(''.join(['ezcawrite ',self.varXChannel,' \'',str(stdDevX),'\'']))
                #os.system(''.join(['ezcawrite ',self.varYChannel,' \'',str(stdDevY),'\'']))
                #os.system(''.join(['ezcawrite ',self.varXYChannel,' \'',str(xyvar),'\'']))

                self.handOffFlag[0] = 1
#End Fitting function
#------------------------------------------------------------
		
#------------------------------------------------------------
#Gui class which calls everything else (Gstreamer, fitting, etc)
class GTK_Main:

    def __init__(self):
        #Initial camera parameters
        
        #Default config file to load
        self.ini_file = 'pcameraSettings.ini'
        #Defaults to using gui windows
        self.guiOn = True
        #Initialize start time
        self.startTime = int(time.time()*1e9)
        #Defaults to not taking a snap shot on startup
        self.takeSnapShotFlag = [False]
        
        #Parse command line
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

        
        if not self.loadSettings(self.ini_file):
            print "Using built-in defaults"
		
            #Using lists of single numbers so these values are "passed by reference"
            #I.e. if the GTK_Main loop's self.height changes, the imageFit thread knows about this
            #However, one needs to be careful exactly when in time these values change and what
            #might be happening as multiples of them change.
            self.cameraUID = 44026
            self.numpyArray = [numpy.array([-1])]
            self.handOffFlag = [1]
            self.computationOn = [True]
            self.HeightWidth = [240,320]
            self.YX = [120,160]
            self.exposure = [50000]
            self.autoExpose = [False]
            self.comXChannel = 'C1:PEM-stacis_EENWY_geo'
            self.comYChannel = 'C1:PEM-stacis_EENWX_geo'
            self.varXChannel = 'C1:PEM-stacis_EESWY_geo'
            self.varYChannel = 'C1:PEM-stacis_EESWX_geo'
            self.varXYChannel = 'C1:PEM-stacis_EESWZ_geo'
            self.UDPmulticastIP = "239.255.1.1"
            self.UDPmulticastPort = 5000
            self.overlayOn = True
            self.jpegSnapFileName = ['./default.jpg']
            self.jpegRunningSnapFlag = [False]
            self.jpegRunningSnapDelay = [10]

        #----------------------------------------------------
        #GUI setup - creating windows and buttons
        if self.guiOn:
            self.window_name = ''.join(['Camera UID ',str(self.cameraUID)])
            window = gtk.Window(gtk.WINDOW_TOPLEVEL)
            window.set_title(self.window_name)
            window.set_default_size(300, -1)
            
            window.connect("destroy",self.quit)
            vbox = gtk.VBox()
            window.add(vbox)
            self.button = gtk.Button("Start")
            self.button.connect("clicked", self.start_stop)
            vbox.add(self.button)

            self.textBoxFileName = gtk.Entry()
            self.textBoxFileName.connect("activate",self.changeFileName)
            vbox.add(self.textBoxFileName)

            self.takeSnapShotButton = gtk.Button(''.join(["Take Snapshot to: ",self.jpegSnapFileName[0].split('/')[-1]]))
            self.takeSnapShotButton.connect("clicked",self.takeSnapShot)
            vbox.add(self.takeSnapShotButton)

            self.textBoxSnapDelay = gtk.Entry()
            self.textBoxSnapDelay.connect("activate",self.changeRunningSnapDelay)
            vbox.add(self.textBoxSnapDelay)

            self.setRunningSnapFlagButton = gtk.Button("Start image capture every 10 seconds")
            self.setRunningSnapFlagButton.connect("clicked",self.setRunningSnapFlag)
            vbox.add(self.setRunningSnapFlagButton)
            
            self.uid32223Button = gtk.Button("Camera 32223")
            self.uid32223Button.connect("clicked",self.cam32223)
            vbox.add(self.uid32223Button)
            
            self.uid44026Button = gtk.Button("Camera 44026")
            self.uid44026Button.connect("clicked",self.cam44026)
            vbox.add(self.uid44026Button)

            self.overlayTextButton = gtk.Button("Overlay Off")
            self.overlayTextButton.connect("clicked",self.overlayText)
            vbox.add(self.overlayTextButton)
            
            self.increaseButton = gtk.Button("Increase Exposure to " + str(self.exposure[0]*2) + " microseconds")
            self.increaseButton.connect("clicked",self.increase_exposure)
            vbox.add(self.increaseButton)
            
            self.decreaseButton = gtk.Button("Decrease Exposure to " + str(self.exposure[0]/2) + " microseconds")
            self.decreaseButton.connect("clicked",self.decrease_exposure)
            vbox.add(self.decreaseButton)

            self.autoExposeOnButton = gtk.Button("Turn Auto Exposure On")
            self.autoExposeOnButton.connect("clicked",self.autoExposeOn)
            vbox.add(self.autoExposeOnButton)
            
            self.saveSettingsButton = gtk.Button("Save Settings")
            self.saveSettingsButton.connect("clicked",self.saveSettings)
            vbox.add(self.saveSettingsButton)
            
            window.show_all()
            #End window setup
            #----------------------------------------------------

        #----------------------------------------------------
        #Start Gstreamer pipeline setup
        self.player = gst.Pipeline("player")
        self.source = pcamerasrc.PCameraSource("source")
        assert self.source
        self.source.set_property("UID",self.cameraUID)

        self.source.set_property("exposure",self.exposure[0])
        self.source.set_property("NumpyArrayReference",self.numpyArray)
        self.source.set_property("NumpyArrayHandOff",self.handOffFlag)
        self.source.set_property("X",self.YX[1])
        self.source.set_property("Y",self.YX[0])
        self.source.set_property("height",self.HeightWidth[0])
        self.source.set_property("width",self.HeightWidth[1])
        self.source.set_property("startTime",self.startTime)
            
        colorspace = gst.element_factory_make("ffmpegcolorspace","colorspace")
        
        self.text = gst.element_factory_make("textoverlay","text0")
        self.text.set_property("valign","top")
        self.text.set_property("halign","left")
        self.text.set_property("text",''.join(['Camera: ',str(self.cameraUID)]))
        queue = gst.element_factory_make("queue","frameQueue")
            
        jpegenc0 = gst.element_factory_make("jpegenc","jpegenc0")
            
        udpsnk = gst.element_factory_make("udpsink","udpsnk")
        udpsnk.set_property("port",self.UDPmulticastPort)
        udpsnk.set_property("host", self.UDPmulticastIP)
        udpsnk.set_property("max-lateness",10000)
        
        self.player.add(self.source,colorspace,queue,self.text,jpegenc0,udpsnk)
        gst.element_link_many(self.source,colorspace,queue,self.text,jpegenc0,udpsnk)
        
        #If no gui, start in the "playing" state
        if not self.guiOn:
            self.player.set_state(gst.STATE_PLAYING)
        #End Gstreamer pipeline setup
        #---------------------------------------------
            
        #---------------------------------------------
        #Image fitting thread start
        self.imageFit = ImageFitThread(self)
        if self.computationOn:
            self.imageFit.start()

        #Image fitting end
        #---------------------------------------------

	#----------------------------------------------------
	#Button definitions for GUI
    def start_stop(self, w):
        if self.button.get_label() == "Start":
            self.button.set_label("Stop")
            self.player.set_state(gst.STATE_PLAYING)
        else:
            self.player.set_state(gst.STATE_NULL)
            self.button.set_label("Start")

    def increase_exposure(self,w):
        self.autoExpose[0] = False
        self.exposure[0] = int(self.exposure[0]*2)
        self.source.set_property("exposure",self.exposure[0])
        self.increaseButton.set_label("Increase Exposure to " + str(self.exposure[0]*2) + " microseconds")
        self.decreaseButton.set_label("Decrease Exposure to " + str(self.exposure[0]/2) + " microseconds")
		
    def decrease_exposure(self,w):
        self.autoExpose[0] = False
        self.exposure[0] = int(self.exposure[0]/2)
        self.source.set_property("exposure",self.exposure[0])
        self.increaseButton.set_label("Increase Exposure to " + str(self.exposure[0]*2) + " microseconds")
        self.decreaseButton.set_label("Decrease Exposure to " + str(self.exposure[0]/2) + " microseconds")
                
    def cam32223(self,w):
        self.source.set_property("UID",32223)

    def cam44026(self,w):
        self.source.set_property("UID",44026)

    def quit(self,w):
        self.computationOn[0] = False
        gtk.main_quit()
        
    def autoExposeOn(self,w):
        if self.autoExposeOnButton.get_label() == "Turn Auto Exposure On":
            self.autoExpose[0] = True
            self.autoExposeOnButton.set_label("Turn Auto Exposure Off")
        else:
            self.autoExpose[0] = False
            self.autoExposeOnButton.set_label("Turn Auto Exposure On")


    def changeFileName(self,textBoxFileName):
        self.jpegSnapFileName[0] = textBoxFileName.get_text()
        self.takeSnapShotButton.set_label(''.join(["Take Snapshot to: ",self.jpegSnapFileName[0].split('/')[-1]]))
        
    def takeSnapShot(self,w):
        self.takeSnapShotFlag[0] = True

    def changeRunningSnapDelay(self,textBoxSnapDelay):
        temp = textBoxSnapDelay.get_text()
        
        try:
            self.jpegRunningSnapDelay[0] = float(temp)
            
        except ValueError:
            print "Could not parse delay text (use format like '123.45')"
            print temp
            print self.jpegRunningSnapDelay[0]
            print "Jpeg delay between images set to default 10 seconds"
            self.jpegRunningSnapDelay[0] = 10
            
        if self.jpegRunningSnapFlag[0] == True:
            self.setRunningSnapFlagButton.set_label(''.join(['Stop taking images every ',
                                                       str(self.jpegRunningSnapDelay[0]),
                                                       ' seconds']))
        else:
            self.setRunningSnapFlagButton.set_label(''.join(['Start taking images every ',
                                                       str(self.jpegRunningSnapDelay[0]),
                                                       ' seconds']))

    def setRunningSnapFlag(self,w):
        if self.jpegRunningSnapFlag[0] == True:
            self.jpegRunningSnapFlag[0] = False
            self.setRunningSnapFlagButton.set_label(''.join(['Start taking images every ',
                                                       str(self.jpegRunningSnapDelay[0]),
                                                       ' seconds']))
        else:
            self.jpegRunningSnapFlag[0] = True
            self.setRunningSnapFlagButton.set_label(''.join(['Stop taking images every ',
                                                       str(self.jpegRunningSnapDelay[0]),
                                                       ' seconds']))
            
    def overlayText(self,w):
        if self.overlayTextButton.get_label() == "Overlay On":
            self.overlayTextButton.set_label("Overlay Off")
            self.overlayOn = True
            self.text.set_property("text","")
        else:
            self.overlayOn = False
            self.overlayTextButton.set_label("Overlay On")

    def saveSettings(self,w):
        try:
            configFileHandle = open(self.configFileName,'w')
            configFileHandle.write(''.join(["UID = ",str(self.cameraUID),'\n']))
            configFileHandle.write(''.join(["Height = ",str(self.HeightWidth[0]),'\n']))
            configFileHandle.write(''.join(["Width = ",str(self.HeightWidth[1]),'\n']))
            configFileHandle.write(''.join(["X = ",str(self.YX[1]),'\n']))
            configFileHandle.write(''.join(["Y = ",str(self.YX[0]),'\n']))
            configFileHandle.write(''.join(["Exposure = ",str(self.exposure[0]),'\n']))
            configFileHandle.write(''.join(["X Center of Mass Channel = ",self.comXChannel,'\n']))
            configFileHandle.write(''.join(["Y Center of Mass Channel = ",self.comYChannel,'\n']))
            configFileHandle.write(''.join(["X Standard Deviation Channel = ",self.varXChannel,'\n']))
            configFileHandle.write(''.join(["Y Standard Deviation Channel = ",self.varYChannel,'\n']))
            configFileHandle.write(''.join(["XY Standard Deviation Channel = ",self.varXYChannel,'\n']))
            configFileHandle.write(''.join(["UDP Multicast IP = ",self.UDPmulticastIP,'\n']))
            configFileHandle.write(''.join(["UDP Multicast Port = ",str(self.UDPmulticastPort),'\n']))
            configFileHandle.write(''.join(["Overlay On = ",str(self.overlayOn),'\n']))
            configFileHandle.close()
        except IOError:
            sys.stderr.write(''.join(['Error opening,closing, or writing to config file: ',self.configFileName,'\n']))
		
    def loadSettings(self,fileName):
        if not os.path.isfile(fileName):
            sys.stderr.write(''.join(['Could not find file: ',str(fileName),'\n']))
            return False
        try:
            configFileHandle = open(fileName,'r')
            self.cameraUID = 32223
            self.numpyArray = [numpy.array([10])]
            self.handOffFlag = [1]
            self.computationOn = [True]
            self.HeightWidth = [240,320]
            self.YX = [120,160]
            self.exposure = [50000]
            self.autoExpose = [False]
            self.comXChannel = 'C1:PEM-stacis_EENWY_geo'
            self.comYChannel = 'C1:PEM-stacis_EENWX_geo'
            self.varXChannel = 'C1:PEM-stacis_EESWY_geo'
            self.varYChannel = 'C1:PEM-stacis_EESWX_geo'
            self.varXYChannel = 'C1:PEM-stacis_EESWZ_geo'
            self.UDPmulticastIP = "239.255.1.1"
            self.UDPmulticastPort = 5000
            self.overlayOn = True
            self.jpegSnapFileName = ['./default.jpg']
            self.jpegRunningSnapFlag = [False]
            self.jpegRunningSnapDelay = [10]
            for line in configFileHandle:
                data = line.split('=',1)
                if len(data) == 2:
                    param = data[0].strip()
                    value = data[1].strip()
                if param == 'UID':
                    self.cameraUID = int(value)
                if param == 'Height':
                    self.HeightWidth[0] = int(value)
                if param == 'Width':
                    self.HeightWidth[1] = int(value)
                if param == 'X':
                    self.YX[0] = int(value)
                if param == 'Y':
                    self.YX[1] = int(value)
                if param == 'Exposure':
                    self.exposure[0] = int(value)
                if param == 'X Center of Mass Channel':
                    self.comXChannel = value
                if param == 'Y Center of Mass Channel':
                    self.comYChannel = value
                if param == 'X Standard Deviation Channel':
                    self.varXChannel = value
                if param == 'Y Standard Deviation Channel':
                    self.varYChannel = value
                if param == 'XY Standard Deviation Channel':
                    self.varXYChannel = value
                if param == 'UDP Multicast IP':
                    self.UDPmulticastIP = value
                if param == 'UDP Multicast Port':
                    self.UDPmulticastPort = int(value)
                if param == 'Overlay On':
                    self.overlayOn = bool(value)
                                            
            return True
        except IOError:
            sys.stderr.write(''.join(['Error opening or reading from config file: ',self.configFileName,'\n']))
            return False
        except ValueError:
            sys.stderr.write(''.join(['Syntax error in config file: ',self.configFileName,'\n']))
            sys.stderr.write(''.join(['Did not understand: ',line,'\n']))
            return False

    
	#Signal handler for nice shutdown when sen

	#def
	#End Button defintions
	#---------------------------------------------------
#End gui class definition
#------------------------------------------------------------


#----------------------------------------------------------
#Get this program running!
GTK_Main()
gtk.gdk.threads_init()
gtk.main()
