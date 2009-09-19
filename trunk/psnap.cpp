//psnap.cpp
//This is a c++ file which contains helper functions to be wrapped by SWIG into python, for a python interface to Prosilica cameras.  Requires the Prosilica API.

//Joseph Betzwieser 2/20/09

//Comment out to remove debug print commands
//#define DEBUG


//Basic Includes
#include "psnap.hpp"

static unsigned char * newFrame = NULL;

/***************************************************************************************************************
Function which assigns the first camera found to the tCamera object it was passed
****************************************************************************************************************/
  
bool getCamera(tCamera* Camera)
{

  tPvUint32 count,connected;
  tPvCameraInfo list;

  count = PvCameraList(&list,1,&connected);
  if(count == 1)
    {
      Camera->UID = list.UniqueId;
      #ifdef DEBUG
      fprintf(stderr, "got camera %s\n",list.SerialString);
      #endif
      return true;
    }
  else
    return false;
}

/***************************************************************************************************************
connect to the camera (aka open or access the camera)
****************************************************************************************************************/


bool connectCamera(tCamera* Camera)
{
	//potrebbe servire e` nel codice di alcuni esempi
	sleep(500);

  tPvErr cam_err;

  cam_err = PvCameraOpen(Camera->UID,ePvAccessMaster,&(Camera->Handle));
  if(cam_err == ePvErrSuccess)
    {
      #ifdef DEBUG
      fprintf(stderr, "Got camera with UID = %lu \n",Camera->UID);
      #endif
      return true;
    }
  else if (cam_err == ePvErrAccessDenied)
    {
      fprintf(stderr, "Access denied to camera, most likely due to another application using it, possibly on another host \n");
      return false;
    }  
  else if (cam_err == ePvErrNotFound)
    {  
      fprintf(stderr, "Unable to find specified camera \n");
      return false;
    }
  else
    {
      fprintf(stderr, "Failed to setup camera for unknown reason \n");
      return false;
    }
}

/***************************************************************************************************************
Sets various parameters on the camera, including height of image, width of image, x position (measured from left side to the left edge of the view area), y position (measured from the top to the top edge of the view area), frame type ('Mono8' or 'Mono16'), packet_size (9000 on jumbo frame enabled networks, 1500 otherwise), exposure time (in microseconds), software gain (in db - I suggest leaving at 0)

If parameters are not within a camera's capabilities, it will send to stderr, and will set to the closest possible value
****************************************************************************************************************/


bool setupCamera(tCamera* Camera, int height_value, int width_value, int x_value, int y_value, char* frame_type, int packet_size, int exposure_time, int gain_value)
{
  
  // Converts values to the tPvUint32 type need by the Prosilica API (which is just an unsigned long)
  unsigned long FrameSize = 0;
  tPvUint32  H = static_cast<unsigned long> (height_value);
  tPvUint32  W = static_cast<unsigned long> (width_value);
  tPvUint32  X = static_cast<unsigned long> (x_value);
  tPvUint32  Y = static_cast<unsigned long> (y_value);
  tPvUint32  exposure = static_cast<unsigned long> (exposure_time);
  tPvUint32  gain = static_cast<unsigned long> (gain_value);
  tPvUint32  Psize = static_cast<unsigned long> (packet_size);

  //Min and max values to be determined from the camera  
  tPvUint32 Hmin = 0, Hmax = 0,Wmin = 0, Wmax = 0, Xmin = 0, Ymin = 0, Xmax = 0, Ymax = 0, GainMin, GainMax,ExpMin,ExpMax;
 
  PvAttrRangeUint32(Camera->Handle,"Width",&Wmin,&Wmax);
  PvAttrRangeUint32(Camera->Handle,"Height",&Hmin,&Hmax);
  PvAttrRangeUint32(Camera->Handle,"RegionX",&Xmin,&Xmax);
  PvAttrRangeUint32(Camera->Handle,"RegionY",&Ymin,&Ymax);
  PvAttrRangeUint32(Camera->Handle,"GainValue",&GainMin,&GainMax);
  PvAttrRangeUint32(Camera->Handle,"ExposureValue",&ExpMin,&ExpMax);


  #ifdef DEBUG
  fprintf(stderr, "Min Width = %lu, Max Width = %lu, Min Height = %lu, Max Height = %lu \n",Wmin,Wmax,Hmin,Hmax);
  #endif
  //Checks if inputs for heigh, width, position are valid
  if ((unsigned long)Xmin > X)
    {
      X = Xmin;
      fprintf(stderr, "Input X must be between %lu and %lu \n",(unsigned long)Xmin,(unsigned long)Xmax);
      fprintf(stderr, "X position has been set to %lu \n", (unsigned long)X);
    }
  if ((unsigned long)Xmax < X)
    {
      X = Xmax;
      fprintf(stderr, "Input X must be between %lu and %lu \n",(unsigned long)Xmin,(unsigned long)Xmax);
      fprintf(stderr, "X position has been set to %lu \n", (unsigned long)X);
    }
 if ((unsigned long)Ymin > Y)
    {
      Y = Ymin;
      fprintf(stderr, "Input Y must be between %lu and %lu \n",(unsigned long)Ymin,(unsigned long)Ymax);
      fprintf(stderr, "Y position has been set to %lu \n", (unsigned long)Y);
    }
  if ((unsigned long)Ymax < Y)
    {
      Y = Ymax;
      fprintf(stderr, "Input Y must be between %lu and %lu \n",(unsigned long)Xmin,(unsigned long)Xmax);
      fprintf(stderr, "Y position has been set to %lu \n", (unsigned long)X);
    }

  if((unsigned long)Wmax < (X + W))
    {
      W = Wmax-Xmin;
      X = Xmin;
      fprintf(stderr, "Input Width + Region of intrest left edge X must be less than or equal to %lu \n",(unsigned long)Wmax);
    }
  if((unsigned long)Hmax < (Y + H))
    {
      H = Hmax-Ymin;
      Y = Ymin;
      fprintf(stderr, "Input Height + Region of interest upper edge Y must be less than or equal to %lu \n",(unsigned long)Hmax);
    }

  #ifdef DEBUG
  fprintf(stderr,"Made it through size checks\n");
  #endif

  if ((unsigned long)exposure < ExpMin)
    {
      X = Xmin;
      fprintf(stderr, "Input exposure must be between %lu and %lu \n",(unsigned long)ExpMin,(unsigned long)ExpMax);
      fprintf(stderr, "Exposure has been set to %lu \n", (unsigned long)X);
    }
  if ((unsigned long)exposure > ExpMax)
    {
      X = Xmin;
      fprintf(stderr, "Input exposure must be between %lu and %lu \n",(unsigned long)ExpMin,(unsigned long)ExpMax);
      fprintf(stderr, "Exposure has been set to %lu \n", (unsigned long)X);
    }
  if ((unsigned long)gain < GainMin)
    {
      X = Xmin;
      fprintf(stderr, "Input gain must be between %lu and %lu \n",(unsigned long)GainMin,(unsigned long)GainMax);
      fprintf(stderr, "Gain has been set to %lu \n", (unsigned long)X);
    }
 if ((unsigned long)gain > GainMax)
    {
      X = Xmin;
      fprintf(stderr, "Input gain must be between %lu and %lu \n",(unsigned long)GainMin,(unsigned long)GainMax);
      fprintf(stderr, "Gain has been set to %lu \n", (unsigned long)X);
    }
 
 //Should fix this - proper thing is to use PvAttrRangeEnum to find
 //valid frame types
 //
 // if ((!strcmp(frame_type, 'Mono8')) || (!strcmp(frame_type, 'Mono16')))
 //  {
 //    frame_type = "Mono8";
 //    fprintf(stderr, "Frame type must be 'Mono8' or 'Mono16' \n",(unsigned long)GainMin,(unsigned long)GainMax);
 //    fprintf(stderr, "Frame type has been set to Mono8 \n");
 //  }
 //

 #ifdef DEBUG
 fprintf(stderr,"Made it through exposure, gain, frametype checks \n");
 #endif

  //JCB add frame type,exposure, gain checks here

 PvAttrUint32Set(Camera->Handle,"Width",W);
 PvAttrUint32Set(Camera->Handle,"Height",H);
 PvAttrUint32Set(Camera->Handle,"RegionX",X);
 PvAttrUint32Set(Camera->Handle,"RegionY",Y);
 PvAttrUint32Set(Camera->Handle,"PacketSize",Psize);
 PvAttrEnumSet(Camera->Handle,"PixelFormat",frame_type);
 PvAttrUint32Set(Camera->Handle,"ExposureValue", exposure);
 PvAttrUint32Set(Camera->Handle,"GainValue", gain);


 //how big should the frame buffers be?
 if(!PvAttrUint32Get(Camera->Handle,"TotalBytesPerFrame",&FrameSize))
   {
     bool failed = false;
     
     // allocate the buffer for the single frame we need
     Camera->Frame.Context[0]  = Camera;
     Camera->Frame.ImageBuffer = new char[FrameSize];
     if(Camera->Frame.ImageBuffer)
       Camera->Frame.ImageBufferSize = FrameSize;
     else
       failed = true;

     #ifdef DEBUG
     fprintf(stderr, "%d is the FrameSize and the Status is %d \n",int(FrameSize),int(Camera->Frame.Status));
     #endif

     if(!failed)
       return true;
     else
       {
	 fprintf(stderr, "Failed to create the new array for the Frame \n");
	 return false;
       }
   }
 else
   {
     fprintf(stderr, "Failed to get back from the camera the size of the Frame array needed \n");
     return false;
   }
}


/***************************************************************************************************************
Start Acquisition!!
****************************************************************************************************************/

bool startCamera(tCamera* Camera)
{
  // set the camera is capture mode
  if(!PvCaptureStart(Camera->Handle))
    {
      // set the camera in continuous acquisition mode
      if(!PvAttrEnumSet(Camera->Handle,"FrameStartTriggerMode","Freerun"))
	{			
	  // and set the acquisition mode into continuous
	  if(PvCommandRun(Camera->Handle,"AcquisitionStart"))
	    {
	      // if that fails, we reset the camera to non capture mode
	      PvCaptureEnd(Camera->Handle) ;
	      return false;
	    }
	  else
	    return true;
	}
      else
	return false;
    }
  else
    return false;
}

/***************************************************************************************************************
Stop streaming
****************************************************************************************************************/

void stopCamera(tCamera* Camera)
{
  PvCommandRun(Camera->Handle,"AcquisitionStop");
  PvCaptureEnd(Camera->Handle);  
}
    
/***************************************************************************************************************
Take a snapshot and save a frame of it from the camera
****************************************************************************************************************/   

bool snapCamera(tCamera* Camera)
{
  int counting = 0;
  if(!PvCaptureQueueFrame(Camera->Handle,&(Camera->Frame),NULL))
    {
      //Waits 400 milliseconds (or whatever the integer number of milliseconds in the 3rd argument is)
      while(PvCaptureWaitForFrameDone(Camera->Handle,&(Camera->Frame),400) == ePvErrTimeout)
	{
	  counting++;
	  fprintf(stderr, "%i cycles waiting\n",counting);
	  if (counting > TIMEOUT_CYCLES)
	    {
	      fprintf(stderr, "Timed out waiting to capture a frame\n");
	      return false;
	    }
	}
      if (Camera->Frame.Status != ePvErrSuccess)
	{
	  fprintf(stderr, "Frame returned with non-successful status, status code is %i \n",Camera->Frame.Status);
	  return false;
	}
      else
	return true;
      
    }
  else
    {
      fprintf(stderr, "failed to enqueue the frame\n");
      return false;
    }
}

/***************************************************************************************************************
Unsetup the camera
****************************************************************************************************************/

void unsetupCamera(tCamera* Camera)
{
  PvCameraClose(Camera->Handle);
  // and free the image buffer of the frame
  delete [] (char*)Camera->Frame.ImageBuffer;
}

/***************************************************************************************************************
Necessary initialization of the Prosilica code for any of the Prosilica dependant code to work
Generally run this once at the beginning, and stopPVAPI() at the end.  Multiple calls of this is ok
****************************************************************************************************************/


bool startPVAPI()
{
  if(!PvInitialize())
    {
	#if DEBUG
		printf("API INITIALIZE SUCCESFUL\n");
	#endif
      return true;
    }
  else
    return false;
}

/***************************************************************************************************************
Uninitializes the Prosilica code - Prosilica dependant code will not work after this
****************************************************************************************************************/

void stopPVAPI()
{
  PvUnInitialize();
}

/***************************************************************************************************************
Initializates the camera structure to all zeros - Seems to be necessary for passing to
Prosilica functions
****************************************************************************************************************/


void allocateCameraStructure(tCamera* Camera)
{
  memset(Camera,0,sizeof(tCamera));
}

/***************************************************************************************************************
Helper function to set the camera UID, for Python wrapping
****************************************************************************************************************/

bool setUID(tCamera * Camera, long camera_uid)
{
  Camera->UID = static_cast<unsigned long>(camera_uid);
  if (Camera->UID == static_cast<unsigned long>(camera_uid))
      return true;
  else 
      return false;
}

/***************************************************************************************************************
Helper function to get the current camera UID, for Python wrapping
****************************************************************************************************************/

unsigned long getUID(tCamera * Camera)
{
  return Camera->UID;
}

/***************************************************************************************************************
DEPRECATED ->  to be modified _. is it good now??

Return the actual frame -> works only in black and white
****************************************************************************************************************/

unsigned char* getFrame(tCamera * Camera,int* s)
{
  int  size = 0;
      if (Camera->Frame.Format == ePvFmtMono8)
	{
		size = Camera->Frame.Width * Camera->Frame.Height;
		if(newFrame == NULL){
			newFrame = new unsigned char [size];
			printf("IMAGE-FRAME-INITIALIZED\n");
		}
		memcpy(newFrame,Camera->Frame.ImageBuffer,size);
		*s = size;
	  return newFrame;
	}
      else if (Camera->Frame.Format == ePvFmtMono16)
	{
	  size = Camera->Frame.Width * Camera->Frame.Height * 2;
		if(newFrame == NULL){
			newFrame = new unsigned char [size];
			printf("IMAGE-FRAME-INITIALIZED\n");
		}
		memcpy(newFrame,Camera->Frame.ImageBuffer,size);
		*s = size;
	  return newFrame;
	}
      else
	{
	  fprintf(stderr,"Unexpected Frame Format - neither Mono8 or Mono16 \n");
	  return NULL;
	}
}
