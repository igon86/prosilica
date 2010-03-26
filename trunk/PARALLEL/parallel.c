#include "fit.h"
#include "parallel.h"

int main(int argc, char* argv[]){
	
	/* MPI Variables */
	int my_rank = 0, p = 0; /* p is the number of processes */
#if ON_DEMAND
	int flag = 0, junk = 0, j = 0;
#endif
	MPI_Status status;

 	/* dimension of cropped image */
	int dim = 0;
#ifndef ON_DEMAND
	/* number of images per worker */
	int num_image = 0;
	/* time variables */
	struct timeval tv1, tv2;	
#endif
	/* Dimension of the cropped image */
	int dimx = 0, dimy = 0;
	
	/* two fits of Gaussian */
	double result [DIM_FIT], fit [DIM_FIT];
	/* image representing Gaussian fit */	
	unsigned char *matrix = NULL;
	/* cropped image taken from a Gaussian image */
	unsigned char *cropped = NULL;

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
						EMITTER
*********************************************************************/		
	
	if(my_rank == EMITTER){
		/* THIS IS THE TASK OF THE EMITTER PROCESS */
#if DEBUG
		printf("EMITTER: RANK %d\n",my_rank);
#endif		
		/* initialization of the fit */
		initialization(argv[1], result, fit, &matrix, &cropped, &dimx, &dimy);
	
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
			printf("EMITTERE IMMAGINE %d\n",i);
			flag = 0;
			while ( !flag ){
				if( MPI_Iprobe( j%(p-PS) + PS , MPI_ANY_TAG , MPI_COMM_WORLD ,&flag, &status) != MPI_SUCCESS) printf("PROBLEMA NELLA IPROBE DELL'EMITTERE\n");
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

		printf("EMITTERE: FINITO DI MANDARE LA TERMINAZIONE...MUORO\n");	
#else			
		/* send the cropped image */
		for(i = p - PS; i < STREAMLENGTH; i++){
			printf("EMITTERE IMMAGINE %d\n", i);
			fflush(stdout);
			MPI_Send(cropped, dim, MPI_UNSIGNED_CHAR, i % (p - PS) + PS, IMAGE, MPI_COMM_WORLD);
			printf("EMITTERE, INVIATA IMMAGINE %d\n", i);
		}
#if MASTER
		printf("MASTER\n");
		gettimeofday(&tv1, NULL);		
		for(i = 0; i < STREAMLENGTH; i++)
			MPI_Recv(fit, DIM_FIT, MPI_DOUBLE, i % (p - PS) + PS, RESULTS, MPI_COMM_WORLD, &status);

		gettimeofday(&tv2, NULL);
		printf("Sono il processo %d (EMITTER master), the completion time: %ld\n", my_rank, (tv2.tv_sec - tv1.tv_sec)*1000000 + tv2.tv_usec - 	tv1.tv_usec);
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

#else
			MPI_Recv(fit, DIM_FIT ,MPI_DOUBLE,i%(p-PS)+PS, RESULTS, MPI_COMM_WORLD, &status);
#endif		
			
#if DEBUG
			printf("IMMAGINE %d %f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", i , fit[PAR_A], fit[PAR_X],
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
#if DEBUG
		printf("WORKER: RANK %d\n",my_rank);
#endif
		/* receive the dimension of the cropped image */
		MPI_Recv(&dimx, 1, MPI_INT, EMITTER, PARAMETERS, MPI_COMM_WORLD, &status);
		MPI_Recv(&dimy, 1, MPI_INT, EMITTER, PARAMETERS, MPI_COMM_WORLD, &status);
		/* receive the fit of the Gaussian */
		MPI_Recv(fit, DIM_FIT, MPI_DOUBLE, EMITTER, RESULTS, MPI_COMM_WORLD, &status);

#if DEBUG
		printf("PROCESSO %d, the initial fit: %f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", my_rank, fit[PAR_A], fit[PAR_X],
		fit[PAR_Y], fit[PAR_SX], fit[PAR_SY], fit[PAR_a], fit[PAR_b], fit[PAR_c]);
#endif
		
		/* define the cropped image */
		dim = dimx * dimy;
		cropped = (unsigned char*) malloc(dim);
#if ON_DEMAND	
		
		while(TRUE){
			MPI_Send(&dim,1,MPI_INT,EMITTER,REQUEST,MPI_COMM_WORLD);
			MPI_Probe(EMITTER,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
			if(status.MPI_TAG == TERMINATION){
				printf("WORKER %d: BECCATO FLAG DI TERMINAZIONE\n",my_rank);
				break;
			}	
			MPI_Recv(cropped, dim, MPI_UNSIGNED_CHAR, EMITTER, IMAGE, MPI_COMM_WORLD, &status);
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
			MPI_Recv(cropped, dim, MPI_UNSIGNED_CHAR, EMITTER, IMAGE, MPI_COMM_WORLD, &status);
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
