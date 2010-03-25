#include "fit.h"
#include "parallel.h"

int main(int argc, char* argv[]){
	
	/* File conteining parameters*/
	FILE* parameters = NULL;

	/* MPI Variables */
	int my_rank = 0, p = 0; /* p is the number of processes */
#if ON_DEMAND
	int flag = 0, junk = 0, j = 0;
#endif
	MPI_Status status;

	/* width and length of the input image */
	int width = 0, length = 0;
	/* parameters fro create the mask */
	int max = 0, min = 0;	
 	/* dimension of cropped image */
	int dim = 0;
#ifndef ON_DEMAND
	/* number of images per worker */
	int num_image = 0;
	/* time variables */
	struct timeval tv1, tv2;	
#endif
	
	/* parameters for the cookie cutter */
	double x0 = 0.0, y0 = 0.0;
	double FWHM_x = 0.0, FWHM_y = 0.0;
	int span_x = 0, span_y = 0;
	int dimx = 0, dimy = 0;
	int x = 0 , y = 0;
	
	/* two fits of Gaussian */
	double result [DIM_FIT], fit [DIM_FIT];
	/* image representing Gaussian fit */	
	unsigned char *matrix = NULL;
	/* cropped image taken from a Gaussian image */
	unsigned char *cropped = NULL;
	/* pixel mask for reduce the dimension of the region to analyze */
	unsigned char *mask = NULL;

	/* indexes */
	int i = 0;

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
#if DEBUG
		printf("EMITTER: RANK %d\n",my_rank);
#endif		
		
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
			fit[i] = 0;
		}

		/* image representing the Gaussian fit */
		matrix = createMatrix( length, width, result);
	
		/* writing the image to be fitted in a FIT file */
#if DEBUG
		writeImage((unsigned char *) matrix,(char *) "gaussiana.tiff", width, length);
#endif	
		maxmin( (unsigned char*) matrix, width, length, &max, &min);

#if DEBUG	
		printf("MAX: %d MIN: %d\n", max, min);
#endif
	
		/* a pixel mask is created in order to reduce the dimensione of the region to analyze with the centroid */
		mask = createMask( (unsigned char*) matrix, width, length, max, min, CROP_PARAMETER);
	
#if DEBUG
		writeImage(mask, (char *) "mask.tiff", width, length);
#endif
	
		centroid(mask, width, length, &x0, &y0, &FWHM_x, &FWHM_y);
	
#if DEBUG
		printf("centro in %f - %f\nCon ampiezza %f e %f\n", x0, y0, FWHM_x, FWHM_y);
#endif
	
		free(mask);
	
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
		cropped = cropImage((unsigned char*) matrix, width, length, x - span_x, x + span_x, y - span_y, y + span_y);
	
#if DEBUG
		writeImage(cropped, (char *) "./CROP.tiff", dimx, dimy);
#endif
	
		/* send to the workers the parameters and images */
		dim = dimx * dimy;
		for(i = PS; i < p; i++){
			MPI_Send(&dimx, 1, MPI_INT, i, PARAMETERS, MPI_COMM_WORLD);
			MPI_Send(&dimy, 1, MPI_INT, i, PARAMETERS, MPI_COMM_WORLD);
			MPI_Send(fit, DIM_FIT, MPI_DOUBLE, i, RESULTS, MPI_COMM_WORLD);
			MPI_Send(cropped, dim, MPI_UNSIGNED_CHAR, i, IMAGE, MPI_COMM_WORLD);
#if DEBUG
			printf("EMITTER: DATI INIZIALI INVIATI A %d\n",i);
#endif
		}
		
#if ON_DEMAND
		printf("ON_DEMAND\n");
		fflush(stdout);

		j = PS;
		for(i = p - PS; i < STREAMLENGTH; i++){
			printf("EMETTITORE IMMAGINE %d\n",i);
			flag = 0;
			while ( !flag ){
				if( MPI_Iprobe( j%(p-PS) + PS , MPI_ANY_TAG , MPI_COMM_WORLD ,&flag, &status) != MPI_SUCCESS) printf("PROBLEMA NELLA IPROBE DELL'EMETTITORE\n");
				fflush(stdout);
				j++;
			}
			printf("MESSAGGIO DA %d \n", j - 1);
			MPI_Recv(&junk, 1, MPI_INT, (j - 1) % (p - PS) + PS, REQUEST, MPI_COMM_WORLD, &status);	
			printf("REQUEST RICEVUTA\n");
			MPI_Send(cropped, dim, MPI_UNSIGNED_CHAR, j % (p - PS) + PS, IMAGE, MPI_COMM_WORLD);
			printf("IMMAGINE MANDATA\n");
		}
		for(i = PS; i < p; i++)
			MPI_Send(NULL, 0, MPI_INT, i, TERMINATION, MPI_COMM_WORLD);

		printf("EMETTITORE: FINITO DI MANDARE LA TERMINAZIONE...MUORO\n");	
#else			
		/* send the cropped image */
		for(i = p - PS; i < STREAMLENGTH; i++){
			printf("EMETTITORE IMMAGINE %d\n", i);
			fflush(stdout);
			MPI_Send(cropped, dim, MPI_UNSIGNED_CHAR, i % (p - PS) + PS, IMAGE, MPI_COMM_WORLD);
			printf("EMETTITORE, INVIATA IMMAGINE %d\n", i);
		}
#if MASTER
		printf("MASTER\n");
		gettimeofday(&tv1, NULL);		
		for(i = 0; i < STREAMLENGTH; i++)
			MPI_Recv(fit, DIM_FIT, MPI_DOUBLE, i % (p - PS) + PS, RESULTS, MPI_COMM_WORLD, &status);

		gettimeofday(&tv2, NULL);
		printf("Sono il processo %d (emettitor master), the completion time: %ld\n", my_rank, (tv2.tv_sec - tv1.tv_sec)*1000000 + tv2.tv_usec - 	tv1.tv_usec);
#endif
#endif				
	}

/*********************************************************************
						COLLETTORE
*********************************************************************/

#ifndef MASTER
	else if(my_rank == COLLECTOR){
		/* THIS IS THE TASK OF THE COLLECTOR PROCESS */
#if DEBUG
		printf("COLLECTOR: RANK %d\n",my_rank);
#endif
		gettimeofday(&tv1,NULL);
		
		for(i = 0; i < STREAMLENGTH; i++){
		
		
#if ON_DEMAND		
			j = 0;
			flag = 0;
			while ( !flag ){
				MPI_Iprobe( (j)%(p-PS)+PS , MPI_ANY_TAG , MPI_COMM_WORLD ,&flag, &status);
				j++;
			}
			MPI_Recv(fit, DIM_FIT ,MPI_DOUBLE, (j-1)%(p-PS)+PS, RESULTS, MPI_COMM_WORLD, &status);
#if DEBUG
			printf("IMMAGINE %d DA WORKER %d %f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", i,(j-1)%(p-PS)+PS , fit[PAR_A], fit[PAR_X],
			 fit[PAR_Y], fit[PAR_SX], fit[PAR_SY], fit[PAR_a], fit[PAR_b], fit[PAR_c]);
#endif

#else
			MPI_Recv(fit, DIM_FIT ,MPI_DOUBLE,i%(p-PS)+PS, RESULTS, MPI_COMM_WORLD, &status);
#endif		
			printf("COLLETTORE: ricevuta immagine %d\n",i);
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
#if DEBUG
		printf("WORKER: RANK %d\n",my_rank);
#endif
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
		
		while(TRUE){
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
			printf("WORKER %d: RICEVUTA IMMAGINE %d\n",my_rank,i);
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
