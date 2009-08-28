#include "mysnap.hpp"

int main(int argc, char* argv[])
{
	//parameter configuration phase
	int iter=1;
	char *dest=NULL;
	int delay=0;
	int parcounter=argc;
	//parsing right to left
	while(parcounter>1){
		char *string = argv[parcounter-2];
		char test=string[1];
		int temp;
		
		//printf("string e`: %s mentre test e`: %c",string,test);
		switch (test){
			case 'f': temp = atoi(argv[parcounter-1]);
				if(temp>0) iter = temp;
				break;
			case 'd': temp = atoi(argv[parcounter-1]);
				if(temp > 0) delay = temp;
				break;
			case 'n': dest=argv[parcounter-1];
				break;
			default: printf("FORMAT IS ./mysnap [ [-d delay] [-f number_of_frames] [-n name_of_the_snapshots] ]\nwithout parameter a single snapshot of name snap is taken\n"); 
				exit(1);
		}
		parcounter = parcounter-2;
	}
	if(dest == NULL) dest="snap";

    // initialise the Prosilica API
    if(!PvInitialize())
    { 
	printf("INIT\n");
	fflush(stdout);
        tCamera Camera;

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
						
						int len = strlen(dest) + 17;
						char file[len];
						file[len-1]='\0';
						
						for (int i=0;i<iter;i++){
							snprintf(file,len,"./Image/%s%03d.tiff",dest,i);
							
							/**direct snap on file tiff*/
							CameraSnapFile(&Camera,file);
							Sleep(delay);
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
