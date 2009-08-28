
//Comment out to remove debug print commands
//#define DEBUG

//Basic Includes
//#include "StdAfx.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#if defined(_LINUX) || defined(_QNX)
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <string>
#include <time.h>
#endif

//Prosilica Camera API includes
#include <PvApi.h>
#include <ImageLib.h>

//Basic constants
#define TIMEOUT_CYCLES 100

//Struct for camera's data
typedef struct 
{
    unsigned long   UID;
    tPvHandle       Handle;
    tPvFrame        Frame;

} tCamera;

// get the first camera found
bool getCamera(tCamera* Camera);

// connect to the camera (aka open the camera)
bool connectCamera(tCamera* Camera);

// setup and start streaming
bool setupCamera(tCamera* Camera, int height_value, int width_value, int x_value, int y_value, char* frame_type, int packet_size, int exposure_time, int gain_set);

//starts acquisition
bool startCamera(tCamera* Camera);

// stop streaming
void stopCamera(tCamera* Camera);

// snap and save a frame from the camera
bool snapCamera(tCamera* Camera);

// unsetup the camera
void unsetupCamera(tCamera* Camera);

//Required PvAPI initialization
bool startPVAPI();

//Uninialize PvAPI drivers to free resources
void stopPVAPI();

//Allocates the memory necessary for camera images
void allocateCameraStructure(tCamera * Camera);

//Set Camera UID
bool setUID(tCamera * Camera, long camera_uid);

//Get Camera UID from a tCamera object
unsigned long getUID(tCamera * Camera);
