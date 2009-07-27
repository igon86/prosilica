#!/usr/bin/env python

import sys,math,Snap

Snap.startPVAPI()

frameType = 'Mono8'
Height = 240
Width = 320
Xpos = 0
Ypos = 0
Exposure = 15000
Gain = 0 #db
PacketSize = 8000


#GC650 ----

testCam = Snap.tCamera()

Snap.allocateCamera(testCam)

Snap.setUID(testCam,32223)

Snap.CameraSetup(testCam)

Snap.CameraStart(testCam, Height, Width, Xpos, Ypos, frameType, PacketSize, Exposure, Gain)

Snap.CameraSnap(testCam)

#GC750 -----

GC750 = Snap.tCamera()

Snap.allocateCamera(GC750)

Snap.setUID(GC750,44026)

Snap.CameraSetup(GC750)

Snap.CameraStart(GC750, Height, Width, Xpos, Ypos, frameType, PacketSize, Exposure, Gain)

Snap.CameraSnap(GC750)


#Snap.CameraSaveTiff(testCam, 'temp.tiff')

#Snap.writeFrameToSTDOUT(testCam,frameType)

Snap.CameraStop(GC750)
Snap.CameraUnsetup(GC750)
Snap.CameraStop(testCam)
Snap.CameraUnsetup(testCam)
Snap.stopPVAPI()
