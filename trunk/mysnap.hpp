#ifndef __MYSNAP__HPP
#define __MYSNAP__HPP

#include "psnap.hpp"

#define MAXSNAP 1000

void Sleep(unsigned int time);

// wait for a camera to be plugged
void WaitForCamera();

// wait for the user to press q
bool WaitForUserToQuitOrSnap();

// get the first camera found
bool CameraGet(tCamera* Camera);

// open the camera
bool CameraSetup(tCamera* Camera);

// setup and start streaming
bool CameraStart(tCamera* Camera);

// stop streaming
void CameraStop(tCamera* Camera);

// snap and save a frame from the camera
void CameraSnap(tCamera* Camera);

void CameraSnapFile(tCamera* Camera, char *dest);

// unsetup the camera
void CameraUnsetup(tCamera* Camera);

#endif
