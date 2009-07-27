#!/usr/bin/env python

#pcamerasrc.py: implements a prosilica camera source element in python
# using psnap.py c++ bindings (basically prosilica PvAPI bindings)

#Joseph Betzwieser 2/4/2009

#Needed imports
import sys
import gobject
import pygst
pygst.require('0.10')
import gst
import psnap
import numpy
import time

class PCameraSource(gst.BaseSrc):
    __gsttemplates__ = (
        gst.PadTemplate("src",
                        gst.PAD_SRC,
                        gst.PAD_ALWAYS,
                        gst.caps_new_any()),
        )



    #Various flags to indicate state of readiness
    cameraRunningFlag = False
    parameterChangeFlag = True
    
    def __init__(self,name):
        self.__gobject_init__()
        psnap.startPVAPI()
        self.myCamera = psnap.tCamera()
        psnap.allocateCameraStructure(self.myCamera)
        #default parameters
        self.set_name(name)
        self.exposure = 10000
        self.height = 240
        self.width = 320
        self.Xpos = 0 #corresponds to width, UL corner, measured from left
        self.Ypos = 0 #corresponds to height, UL corner, measured from top
        self.frameType = 'Mono8'
        self.depth = '8'
        self.gain = 0
        self.cameraUID = None 
        self.cameraIP = None
        self.packetSize = 9000
        self.cameraRunningFlag = False
        self.parameterChangeFlag = True
        self.numpyArray = numpy.array([0])
        self.handOff = [0]
        self.colorMode = 'Grey'
        self.colorDepth = '8'
        self.colorBPP = '8'
        self.frameCount = 1000

        self.set_live(True)

    #Handles the setting of camera parameters
    def set_property(self, name, value):
        self.parameterChangeFlag = True
        if name == 'exposure':
            self.exposure = value
        elif name == 'height':
            self.height = value
        elif name == 'width':
            self.width = value
        elif name == 'UID':
            self.cameraUID = value
        elif name == 'X':
            self.Xpos = value
        elif name == 'Y':
            self.Ypos = value
        elif name == 'frameType':
            self.frameType = value
            if frameType == 'Mono8':
                self.depth = '8'
            elif frameType == 'Mono16':
                self.depth = '16'
        elif name == 'gain':
            self.gain = value
        elif name == 'packetSize':
            self.PacketSize = value
        elif name == 'reset':
            self.parameterChangeFlag = True
        elif name == 'NumpyArrayReference':
            self.numpyArray = value
        elif name == 'NumpyArrayHandOff':
            self.handOff = value

        #FIXME: Need some better error handling - i.e. how to send 
        #approriate messages other than unexpected
    def do_create(self, offset, size):
        #Are we running and need to change something? If yes, stop
        if self.cameraRunningFlag and self.parameterChangeFlag:
            psnap.stopCamera(self.myCamera)
            psnap.unsetupCamera(self.myCamera)
            self.cameraRunningFlag = False
        #Do we have things to change?
        if self.parameterChangeFlag:
            #Is there an actual UID (camera ID)?
            if (self.cameraUID != None):
                psnap.setUID(self.myCamera, self.cameraUID)
            else:
                print "Failed to set UID"
                return gst.FLOW_UNEXPECTED, None
            #Grab the camera with the now set UID
            if not psnap.connectCamera(self.myCamera):
                #Failed to setup camera
                print "Failed to setup camera"
                return gst.FLOW_UNEXPECTED, None
        if self.cameraRunningFlag == False:
            #Start the camera with current parameters
            if psnap.setupCamera(self.myCamera, self.height,
                                self.width, self.Xpos, self.Ypos, 
                                self.frameType, self.packetSize, 
                                self.exposure, self.gain):
                if psnap.startCamera(self.myCamera):
                    self.cameraRunningFlag = True
                    self.parameterChangeFlag = False
                else:
                    print "Failed to start camera"
                    return gst.FLOW_UNEXPECTED, None
            else:
                print "Failed to Setup Camera"
                return gst.FLOW_UNEXPECTED, None
        #We're running and have everything updated 
        #Just take a picture
        if psnap.snapCamera(self.myCamera):
            imageFrame = psnap.getFrame(self.myCamera,self.colorMode)
            #Did it work? If yes, make a gst.Buffer and send it
            if imageFrame:
                if (self.handOff[0] == 1):
                    if self.frameType == 'Mono8':
                        self.numpyArray[0] = numpy.frombuffer(imageFrame,dtype=numpy.uint8).copy()
                        self.numpyArray[0].shape = [self.height,self.width]
                    if self.frameType == 'Mono16':
                        self.numpyArray[0] = numpy.frombuffer(imageFrame,dtype=numpy.uint16).copy()
                        self.numpyArray[0].shape = [self.height,self.width]
                    self.handOff[0] = 0
                sys.stdout.flush()
                imageBuffer = gst.Buffer(imageFrame)
                imageBuffer.duration = int(self.exposure*1000) #convert to nanoseconds from microseconds
                caps = gst.caps_from_string(''.join(["video/x-raw-gray, height=",str(self.height),
                                                     ", width=",str(self.width),", bpp=",self.colorBPP,
                                                     ",depth=",self.colorDepth,", framerate=0/1"]))
                imageBuffer.set_caps(caps)
                
                ##############################################
                #Timed section -timestamp for each 1000 frames
                self.frameCount += 1
                if self.frameCount >= 1000:
                    print time.time()
                    self.frameCount = 0
                
                return gst.FLOW_OK, imageBuffer
            else:
                print "imageFrame does not exist"
                return gst.FLOW_UNEXPECTED, None
        else:
            #Now try up to 10 times to get the image again
            for tries in range(1,10):
                print ''.join(["Failed to get frame - try number: ", str(tries)])
                if psnap.snapCamera(self.myCamera):
                    imageFrame = psnap.getFrame(self.myCamera,self.colorMode)
                    #Did it work? If yes, make a gst.Buffer and send it
                    if imageFrame:
                        if (self.handOff[0] == 1):
                            if self.frameType == 'Mono8':
                                self.numpyArray[0] = numpy.frombuffer(imageFrame,dtype=numpy.uint8).copy()
                                self.numpyArray[0].shape= [self.height,self.width]
                            if self.frameType == 'Mono16':
                                self.numpyArray[0] = numpy.frombuffer(imageFrame,dtype=numpy.uint16).copy()
                                self.numpyArray[0].shape = [self.height,self.width]
                            self.handOff[0] = 0
                        imageBuffer = gst.Buffer(imageFrame)
                        imageBuffer.duration = int(self.exposure * 1000) #convert to nanoseconds from microseconds
                        caps = gst.caps_from_string(''.join(["video/x-raw-gray, height=",str(self.height),
                                                             ", width=",str(self.width),", bpp=",self.colorBPP,
                                                             ",depth=",self.colorDepth,", framerate=0/1"]))
                        imageBuffer.set_caps(caps)
                        return gst.FLOW_OK, imageBuffer
                    else:
                        print "imageFrame does not exist"
                        return gst.FLOW_UNEXPECTED, None
            #We've failed 10 times - try to reset the camera
            psnap.stopCamera(self.myCamera)
            psnap.unsetupCamera(self.myCamera)
            self.cameraRunningFlag = False
            
            #Grab the camera again
            if not psnap.connectCamera(self.myCamera):
                #Failed to setup camera
                print "Failed to setup camera"
                return gst.FLOW_UNEXPECTED, None
            #Start the camera with current parameters
            if psnap.setupCamera(self.myCamera, self.height,
                                self.width, self.Xpos, self.Ypos, 
                                self.frameType, self.packetSize, 
                                self.exposure, self.gain):
                if psnap.startCamera(self.myCamera):
                    self.cameraRunningFlag = True
                    self.parameterChangeFlag = False
                else:
                    print "Failed to start camera"
                    return gst.FLOW_UNEXPECTED, None
            else:
                print "Failed to Setup Camera"
                return gst.FLOW_UNEXPECTED, None
            #Now try up to 10 more times to get the image again
            for tries in range(1,10):
                print ''.join(["Failed to get frame - try number: ", str(tries)])
                if psnap.snapCamera(self.myCamera):
                    imageFrame = psnap.getFrame(self.myCamera,self.colorMode)
                    #Did it work? If yes, make a gst.Buffer and send it
                    if imageFrame:
                        if (self.handOff[0] == 1):
                            if self.frameType == 'Mono8':
                                self.numpyArray[0] = numpy.frombuffer(imageFrame,dtype=numpy.uint8).copy()
                                self.numpyArray[0].shape= [self.height,self.width]
                            if self.frameType == 'Mono16':
                                self.numpyArray[0] = numpy.frombuffer(imageFrame,dtype=numpy.uint16).copy()
                                self.numpyArray[0].shape = [self.height,self.width]
                            self.handOff[0] = 0
                        imageBuffer = gst.Buffer(imageFrame)
                        imageBuffer.duration = int(self.exposure * 1000) #convert to nanoseconds from microseconds
                        caps = gst.caps_from_string(''.join(["video/x-raw-gray, height=",str(self.height),
                                                             ", width=",str(self.width),", bpp=",self.colorBPP,
                                                             ",depth=",self.colorDepth,", framerate=0/1"]))
                        imageBuffer.set_caps(caps)
                        return gst.FLOW_OK, imageBuffer
                    else:
                        print "imageFrame does not exist"
                        return gst.FLOW_UNEXPECTED, None
            print "Camera Snap failed"
            return gst.FLOW_UNEXPECTED, None
     
gobject.type_register(PCameraSource)
