#include <mpi.h>
#include <sys/time.h>
#include "fit.h"

#define OUTPUT_MATRIX "gaussiana.tiff" 
#define CROP_PARAMETER 0.5

/* length of the stream */
#define STREAMLENGTH 4

#define EMETTITOR 0 // rank emettitor
#if MASTER
	#define COLLECTOR 0 // rank collector
#else
	#define COLLECTOR 1
#endif
	
// number of service processes
#if MASTER
	#define PS 1       
#else
	#define PS 2
#endif

/* macro for define MPI tags of the messages */
#define PARAMETERS 0
#define IMAGE 1
#define RESULTS 2
#define REQUEST 3
#define TERMINATION 4

int main(int argc, char* argv[]){
	
	/* File conteining parameters*/
	FILE* parameters;

	/* MPI Variables */
	int my_rank, p, flag, junk; // p is the number of processes
	MPI_Status status;

	/* width and length of the input image */
	int width, length;
	/* parameters fro create the mask */
	int max, min;	
 	/* dimension of cropped image */
	int dim;
	/* number of images per worker */
	int num_image;

	/* time variables */
	struct timeval tv1, tv2;
	
	/* parameters for the cookie cutter */
	double x0, y0;
	double FWHM_x, FWHM_y;
	int span_x, span_y;
	int dimx, dimy;
	int x, y;
	
	/* two fits of Gaussian */
	double result [DIM_FIT], fit [DIM_FIT];
	/* image representing Gaussian fit */	
	unsigned char *matrix;
	/* cropped image taken from a Gaussian image */
	unsigned char *cropped;
	
	/* indexes */
	int i, j;

	/* check the input parameters */
	if(argc != 2){
		fprintf(stderr, "Invalid number of parameters\n");
		exit(EXIT_FAILURE);
	}

	/* Initialize of MPI */
	MPI_Init (&argc, &argv);
	/* Every process takes the own rank*/
	MPI_Comm_rank (MPI_COMM_WORLD, &my_rank);
	/* Total number of processes */
	MPI_Comm_size (MPI_COMM_WORLD, &p);

/*********************************************************************
						EMETTITORE
*********************************************************************/		
	
	if(my_rank == EMETTITOR){
		/* THIS IS THE TASK OF THE EMETTITOR PROCESS */
		
		/* reading the input parameters */		
		if((parameters = fopen(argv[1], "r")) == NULL){
			fprintf(stderr, "File not valid");
			exit(EXIT_FAILURE);		
		}
	
		/* initialize the dimension of the image */
		if(fscanf(parameters, "%d\t%d\t", &width, &length) == 0){
			fprintf(stderr, "File not valid");
			exit(EXIT_FAILURE);							
		}
		/* initialize the fit of the Gaussian */
		for(i = 0; i < DIM_FIT; i++){
			if(fscanf(parameters, "%lf\t", &result[i]) == 0){
				fprintf(stderr, "File not valid");
				exit(EXIT_FAILURE);							
			}		
			fit[i]=0;
		}

		/* image representing the Gaussian fit */
		matrix = createMatrix( length, width, result);
	
		/* writing the image to be fitted in a FIT file */
#if DEBUG
		writeImage((unsigned char *) matrix,(char *) OUTPUT_MATRIX, width, length);
#endif	
		maxmin( (unsigned char*) matrix, width, length, &max, &min);

#if DEBUG	
		printf("MAX: %d MIN: %d\n", max, min);
#endif
	
		/* a pixel mask is created in order to reduce the dimensione of the region to analyze with the centroid */
		unsigned char *mask = createMask( (unsigned char*) matrix, width, length, max, min, CROP_PARAMETER);
	
#if DEBUG
		writeImage(mask, (char *) "mask.tiff", width, length);
#endif
	
		centroid(mask, width, length, &x0, &y0, &FWHM_x, &FWHM_y);
	
#if DEBUG
		printf("centro in %f - %f\nCon ampiezza %f e %f\n", x0, y0, FWHM_x, FWHM_y);
#endif
	
		delete mask;
	
		/* inizialization for the diameter of the gaussian*/
		span_x = (int) (2 * FWHM_x);
		span_y = (int) (2 * FWHM_y);
	
		/* determination of the dimension of the crop */
		dimx = 2 * span_x + 1;
		dimy = 2 * span_y + 1;
	
		/* inizialization of the position coordinates */
		x = (int) x0;
		y = (int) y0;
	
		/**
		 inizialization of the fit of the Gaussian
		 NOTE: the coordinates of the position (x,y) are relative to the cropped portion of the image.
		 The value of span_x, which is approximately the diameter of the gaussian, is generally not as bad
		 as you may think to start the fit.
		 */
		fit[PAR_A] = max;
		fit[PAR_X] = span_x;
		fit[PAR_Y] = span_y;
		fit[PAR_SX] = FWHM_x;
		fit[PAR_SY] = FWHM_y;
		fit[PAR_c] = min;	

#if DEBUG
	printf("%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", fit[PAR_A], fit[PAR_X] + x - span_x, fit[PAR_Y] + y - span_y, fit[PAR_SX], fit[PAR_SY], fit[PAR_a], fit[PAR_b], fit[PAR_c]);
#endif
	
		/* THIS PART CAN BE ITERATIVE */
	
		unsigned char *cropped = cropImage((unsigned char*) matrix, width, length, x - span_x, x + span_x, y - span_y, y + span_y);
	
#if DEBUG
		writeImage(cropped, (char *) "./CROP.tiff", dimx, dimy);
#endif
	
		// send to the workers the parameters
		dim = dimx * dimy;
		for(i = PS; i < p; i++){
			MPI_Send(&dimx, 1, MPI_INT, i, PARAMETERS, MPI_COMM_WORLD);
			MPI_Send(&dimy, 1, MPI_INT, i, PARAMETERS, MPI_COMM_WORLD);
			MPI_Send(fit, DIM_FIT, MPI_DOUBLE, i, RESULTS, MPI_COMM_WORLD);
		}
		// prima mandata di immagini
		for(i=PS;i<p;i++){
			MPI_Send(cropped, dim, MPI_UNSIGNED_CHAR, i%(p-PS)+PS, IMAGE, MPI_COMM_WORLD);
		}
		
#if ON_DEMAND
		printf("ON_DEMAND\n");
		j=PS;
		for(i=p-PS; i < STREAMLENGTH; i++){
			printf("EMETTITORE IMMAGINE %d\n",i);
			flag = 0;
			while ( !flag ){
				MPI_Iprobe( (j)%(p-PS)+PS , MPI_ANY_TAG , MPI_COMM_WORLD ,&flag, &status);
				j++;
			}
			printf("MESSAGGIO DA %d \n",j-1);
			MPI_Recv(&junk, 1, MPI_INT, (j-1)%(p-PS)+PS, REQUEST, MPI_COMM_WORLD,&status);	
			printf("REQUEST RICEVUTA\n");
			MPI_Send(cropped, dim, MPI_UNSIGNED_CHAR, j%(p-PS)+PS, IMAGE, MPI_COMM_WORLD);
			printf("IMMAGINE MANDATA\n");
		}
		for(i=PS;i<p;i++){
			MPI_Send(NULL,0,MPI_INT,i,TERMINATION,MPI_COMM_WORLD);
		}
		printf("EMETTITORE: FINITO DI MANDARE LA TERMINAZIONE...MUORO\n");	
#else			
		// send the cropped image
		for(i=p-PS; i < STREAMLENGTH; i++)
			MPI_Send(cropped, dim, MPI_UNSIGNED_CHAR, i%(p-PS)+PS, IMAGE, MPI_COMM_WORLD);
			
	#if MASTER
		printf("MASTER\n");
		for(i=0; i < STREAMLENGTH; i++)
			MPI_Recv(fit, DIM_FIT, MPI_DOUBLE, i%(p-PS)+PS, RESULTS, MPI_COMM_WORLD,&status);	
	#endif
#endif				
	}

/*********************************************************************
						COLLETTORE
*********************************************************************/

#ifndef MASTER
	else if(my_rank == COLLECTOR){
		/* THIS IS THE TASK OF THE COLLECTOR PROCESS */

		gettimeofday(&tv1,NULL);
		
		for(i=0; i < STREAMLENGTH; i++){
		
		
#if ON_DEMAND		
			j=0;
			flag = 0;
			while ( !flag ){
				MPI_Iprobe( (j)%PS+PS , MPI_ANY_TAG , MPI_COMM_WORLD ,&flag, &status);
				j++;
			}
			MPI_Recv(fit, DIM_FIT ,MPI_DOUBLE, (j-1)%(p-PS)+PS, RESULTS, MPI_COMM_WORLD, &status);
#else
			MPI_Recv(fit, DIM_FIT ,MPI_DOUBLE,i, RESULTS, MPI_COMM_WORLD, &status);
#endif		

		
			printf("COLLETTORE: ricevuta immagine %d\n",i);
#if DEBUG
			printf("IMMAGINE %d DA WORKER %d %f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", i,(j-1)%(p-PS)+PS , fit[PAR_A], fit[PAR_X],
			 fit[PAR_Y], fit[PAR_SX], fit[PAR_SY], fit[PAR_a], fit[PAR_b], fit[PAR_c]);
#endif


		}
		gettimeofday(&tv2,NULL);
		printf("Sono il processo %d (collector), the completion time: %ld\n", my_rank, (tv2.tv_sec - tv1.tv_sec)*1000000 + tv2.tv_usec - tv1.tv_usec);
	}
#endif	

/*********************************************************************
							WORKER
*********************************************************************/
	else{
		/* THIS IS THE TASK OF THE WORKER PROCESS */

		/* receive the dimension of the cropped image */
		MPI_Recv(&dimx, 1, MPI_INT, EMETTITOR, PARAMETERS, MPI_COMM_WORLD, &status);
		MPI_Recv(&dimy, 1, MPI_INT, EMETTITOR, PARAMETERS, MPI_COMM_WORLD, &status);
		/* receive the fit of the Gaussian */
		MPI_Recv(fit, DIM_FIT, MPI_DOUBLE, EMETTITOR, RESULTS, MPI_COMM_WORLD, &status);

#if DEBUG
		printf("PROCESSO %d, the initial fit: %f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", my_rank, fit[PAR_A], fit[PAR_X],
		fit[PAR_Y], fit[PAR_SX], fit[PAR_SY], fit[PAR_a], fit[PAR_b], fit[PAR_c]);
#endif
		
		/* define the cropped image */
		dim = dimx * dimy;
		cropped = (unsigned char*) malloc(dim);
#if ON_DEMAND	
		
		while(true){
			MPI_Send(&dim,1,MPI_INT,EMETTITOR,REQUEST,MPI_COMM_WORLD);
			MPI_Probe(EMETTITOR,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
			if(status.MPI_TAG == TERMINATION){
				printf("WORKER %d: BECCATO FLAG DI TERMINAZIONE\n",my_rank);
				break;
			}	
			MPI_Recv(cropped, dim, MPI_UNSIGNED_CHAR, EMETTITOR, IMAGE, MPI_COMM_WORLD, &status);
			/* image procedure */
			procedure (cropped, dimx, dimy, fit);
			
			MPI_Send(fit, DIM_FIT, MPI_DOUBLE, COLLECTOR , RESULTS, MPI_COMM_WORLD);
		}

		printf("WORKER %d: MUORO\n",my_rank);
#else	
		/* receive the number of the images */
		num_image = STREAMLENGTH / (p - PS);		
		if(STREAMLENGTH % (p - PS) > (my_rank - PS))
			num_image++;

	#if DEBUG		
		printf("Sono il processo %d  e devo ricevere %d immagini\n", my_rank, num_image);
	#endif		
		/* work on the images and send them to the collector */
		for (i = 0; i < num_image ; i++){
			MPI_Recv(cropped, dim, MPI_UNSIGNED_CHAR, EMETTITOR, IMAGE, MPI_COMM_WORLD, &status);
			/* image procedure */
			procedure (cropped, dimx, dimy, fit);

			MPI_Send(fit, DIM_FIT, MPI_DOUBLE, COLLECTOR , RESULTS, MPI_COMM_WORLD);
		}
#endif		
	}
	
	/* Finalize of MPI */
	MPI_Finalize();

	return 0;
}
