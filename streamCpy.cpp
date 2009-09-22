#include <stdio.h>

#include "psnap.hpp"
#include "mysnap.hpp"
#include "tiffPostElaboration.hpp"
#include "fit.hpp" 

#define iter 0
#define len 30

extern FILE* fitDebug;


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
						
						/* Frame parameters*/
						int max;
						int min;
						
						/*centroid variables*/
						double x0;
						double y0;
						double FWHM_x;
						double FWHM_y;
						
						/*centroid varibles (rounded)*/
						int x;
						int y;
						int span_x;
						int span_y;
						
						
						/* dimension of the crop*/
						int dimx;
						int dimy;
						
						/* Gaussian fit struct*/
						fit_t test_g;
						test_g.type = GAUSSIAN;

						
						/*local frame buffer */
						unsigned char* image;
						
						/* Dimension of the image -> not supposed to change*/
						int w = (&Camera)->Frame.Width;
						int h = (&Camera)->Frame.Height;
						
						/** COOKIE CUTTER*/
						
						/**snap a picture*/
						snapCamera(&Camera);
							
						/**get the frame*/
						image = getFrame(&Camera,&tempsize);
						
						maxmin(image,w,h,&max,&min);
							
						printf("MAX: %d MIN: %d\n",max,min);
							
						unsigned char* mask = createMask(image,w,h,max,min,0.5);
							
#if DEBUG
						writeImage(mask,"mask.tiff",w,h);
#endif
							
						centroid(mask,w,h,&x0,&y0,&FWHM_x,&FWHM_y);
							
#if DEBUG
						printf("centro in %f - %f\nCon ampiezza %f e %f\n",x0,y0,FWHM_x,FWHM_y);
#endif

						delete mask;
							
						span_x = (int) (2*FWHM_x);
						span_y = (int) (2*FWHM_y);
						x = (int) x0;
						y = (int) y0;
						
						/*struct inizialization*/
						test_g.A = max;
						test_g.x_0 = span_x;
						test_g.y_0 = span_y;
						test_g.sigma_x = FWHM_x;
						test_g.sigma_y = FWHM_y;
						test_g.a = 0;
						test_g.b = 0;
						test_g.c = min;
						
						dimx = 2*span_x+1;
						dimy = 2*span_y+1;

						int i=0;
						while(true){
#if DEBUG
							fprintf(fitDebug,"\n\nIMMAGINE: %d\n\n",i);
#endif
							i++;
							printf("IMAGE: %d\n",i);
							/**snap a picture*/
							snapCamera(&Camera);
							
							/**get the frame*/
							image = getFrame(&Camera,&tempsize);
							
#if DEBUG
							sprintf(file,"./Image/TEST%03d.tiff",i);
							writeImage(image,file,w,h);
#endif
							
							/** REAL TIME FITTING */
							
							/* local creation of the crop*/
							unsigned char* cropped = cropImage(image,w,h,x-span_x,x+span_x,y-span_y,y+span_y);
							
							
#if DEBUG					
							sprintf(file,"./Image/CROP%03d.tiff",i);
							writeImage(cropped,file,dimx,dimy);
#endif
							

#if DEBUG	
							int dimension = dimx*dimy;
							
							unsigned char* pred = new unsigned char [dimension];
							int temp;
							for (int j=0; j < dimension; j++){
								temp = (int) evaluateGaussian(&test_g,j%dimx,j/dimx);
								pred[j] = temp;
							}
							writeImage(pred,"pred.tiff",dimx,dimy);
#endif
							
							iteration(cropped,dimx,dimy,&test_g);
							
#if DEBUG
							unsigned char* fit = new unsigned char [dimension];
							for (int j=0; j < dimension; j++){
								temp = (int) evaluateGaussian(&test_g,j%dimx,j/dimx);
								pred[j] = temp;
							}
							writeImage(pred,"fit.tiff",dimx,dimy);
#endif
			
							/** ADJUSTMENT FOR THE CROP -> adjustment on the test_g?
							span_x = (int) (2*test_g.sigma_x);
							span_y = (int) (2*test_g.sigma_y);
							x = (int) test_g.x_0;
							y = (int) test_g.y_0;
							*/
							
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
