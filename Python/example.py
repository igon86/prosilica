#!/usr/bin/env python

import sys
import os
import psnap
import numpy

#Allows you to use the prosillica code which has been wrapped by psnap
psnap.startPVAPI()

#Generate camera structure to hold camera unique ID as well as frame buffer
myCamera = psnap.tCamera()

#Unique camera ID for the GC750
UID = 44026

#Set the UID for the camera structure
psnap.setUID(myCamera,UID)


#Have code connect to camera - now you can interact and set values
psnap.connectCamera(myCamera)

#Necessary parameters for the camera to know what size, exposure, etc for the frame it is going to take
exposure = 10000 #In microseconds
height = 240
width = 320
Xpos = 0 #corresponds to width, UL corner, measured from left
Ypos = 0 #corresponds to height, UL corner, measured from top
frameType = 'Mono8' #Monochrome, 8 bit numbers
packet_size = 9000 #MTU setting
gain = 0 #Gain in dB, 0 to 20

#Sets the parameters for the camera, so it knows what kind of data to take
psnap.setupCamera(myCamera, height, width, Xpos, Ypos, frameType, packet_size, exposure, gain)

#Starts the camera in continuous acquisition mode
psnap.startCamera(myCamera)

#Actually takes an image
psnap.snapCamera(myCamera)

#Dumps the frame to a python buffer
imageFrame = psnap.getFrame(myCamera,'Grey')

#Stops camera acquisition 
psnap.stopCamera(myCamera)

#Releases control of the camera
psnap.unsetupCamera(myCamera)


#Stops the background code which allows you to use the prosilica wrapped code
psnap.stopPVAPI()
