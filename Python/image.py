#!/usr/bin/env python

import sys
import os
import psnap
import numpy
import scipy
import matplotlib
import time
from matplotlib import pyplot
from pylab import *

#Allows you to use the prosillica code which has been wrapped by psnap
psnap.startPVAPI()

#Generate camera structure to hold camera unique ID as well as frame buffer
myCamera = psnap.tCamera()

#Unique camera ID for the GC650
UID = 32223

#Set the UID for the camera structure
psnap.setUID(myCamera,UID)

time.sleep(1)

#Have code connect to camera - now you can interact and set values
psnap.connectCamera(myCamera)

time.sleep(1)

#Necessary parameters for the camera to know what size, exposure, etc for the frame it is going to take
exposure = 10000 #In microseconds
height = 240
width = 320
Xpos = 160 #corresponds to width, UL corner, measured from left
Ypos = 120 #corresponds to height, UL corner, measured from top
frameType = 'Mono8' #Monochrome, 8 bit numbers
packet_size = 9000 #MTU setting
gain = 0 #Gain in dB, 0 to 20

#Sets the parameters for the camera, so it knows what kind of data to take
psnap.setupCamera(myCamera, height, width, Xpos, Ypos, frameType, packet_size, exposure, gain)

time.sleep(1)

#Starts the camera in continuous acquisition mode
psnap.startCamera(myCamera)

#Actually takes an image
psnap.snapCamera(myCamera)

#Dumps the frame to a python buffer
imageFrame = psnap.getFrame(myCamera,'Grey')

#Converts to numpy array and Reshapes 
imageArray = numpy.frombuffer(imageFrame,dtype=numpy.uint8).copy().reshape(240,320)

#Saves image as a .png file
#matplotlib.pyplot.imagesave("image1.png", imageFrame, format=png)

#Displays image
matplotlib.pyplot.imshow(imageArray)
show(imageFrame)

#Stops camera acquisition 
psnap.stopCamera(myCamera)

#Releases control of the camera
psnap.unsetupCamera(myCamera)

#Stops the background code which allows you to use the prosilica wrapped code
psnap.stopPVAPI()


