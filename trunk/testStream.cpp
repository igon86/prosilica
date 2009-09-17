#include <stdio.h>

#include "psnap.hpp"
#include "mysnap.hpp"
#include "tiffPostElaboration.hpp"

#define iter 100
#define len 30

int main(int argc, char* argv[])
{
    // initialise the Prosilica API
    if(!PvInitialize())
    { 
	printf("INIT\n");
	fflush(stdout);
	tCamera Camera;

	/* this is basically taken from example.py */

        memset(&Camera,0,sizeof(tCamera));

        // wait for a camera to be plugged
        WaitForCamera();
		
        // get a camera from the list
        if(CameraGet(&Camera))
        {
            // setup the camera
            if(CameraSetup(&Camera))
            {
                // strat streaming from the camera
                if(CameraStart(&Camera))
                {
                    printf("camera is ready now. Press q to quit or s to take a STREAM of pictures\n");
                    // wait for the user to quit or snap
                    if(WaitForUserToQuitOrSnap())
                    {
						clock_t start,end;
						start = clock();
						
						char file[len];
						file[len-1]='\0';
						
						//INIZIALIZATION SHOT -> it comes out bad for some reason
						snapCamera(&Camera);
						
						//FIT VARIABLES
						/* frame dimension*/
						int tempsize;
						
						
						for (int i=0;i<iter;i++){
							snprintf(file,len,"./Image/stream%03d.tiff",i);
							int w = (&Camera)->Frame.Width;
							int h = (&Camera)->Frame.Height;

							/**direct snap on file tiff*/
							snapCamera(&Camera);
							
							unsigned char* temp = getFrame(&Camera,&tempsize);

							/**write the test image*/
							printf("\n\n%s\n",file);
							writeImage(temp,file,w,h);

							//Sleep(delay);
							
							/** REAL TIME FITTING */
							
							//unsigned char* mask = createMask(immagine,w,h,max,min,0.5);
							
							
							//centroid(mask,w,h,&x0,&y0,&FWHM_x,&FWHM_y);
						}
						end = clock();
						double diff = ((double) (end - start))/ CLOCKS_PER_SEC;
						//maybe buggy in some platforms
						printf("Elapsed time: %f\n",diff);
                    }

                    // stop the streaming
                    CameraStop(&Camera);
                }
                else
                    printf("failed to start streaming\n");

                // unsetup the camera
                CameraUnsetup(&Camera);
            }
            else
                printf("failed to setup the camera\n");
        }
        else
            printf("failed to find a camera\n");
       
        // uninitialise the API
        PvUnInitialize();
    }
    else
        printf("failed to initialise the API\n");
    
    return 0;
}
