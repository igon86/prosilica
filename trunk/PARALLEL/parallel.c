#include "fit.h"
#include "parallel.h"

int my_rank;

int main(int argc, char* argv[]){
	
	/* MPI Variables */
	int p = 0; /* p is the number of processes */
#ifdef ON_DEMAND
	int flag = 0, junk = 0;
#endif
	MPI_Status status;
 	/* dimension of cropped image */
	int dim = 0;
#ifndef ON_DEMAND
	/* number of images per worker */
	int num_image = 0;	
#endif
	/* time variables */
	struct timeval tv1, tv2;
	/* Dimension of the cropped image */
	int dimx = 0, dimy = 0;
	
	/* two fits of Gaussian */
	double input [DIM_FIT], fit [DIM_FIT];
	/* image representing Gaussian fit */	
	unsigned char *matrix = NULL;
	/* cropped image taken from a Gaussian image */
	unsigned char *cropped = NULL;

	/* indexes */
	int i = 0, j = 0;

	/* data for the LU solver */
	double* data = (double*) malloc(sizeof(double) * DIM_FIT * (DIM_FIT + 1));
	gsl_matrix_view matrice = gsl_matrix_view_array(data, DIM_FIT, DIM_FIT);
	gsl_vector_view vettore = gsl_vector_view_array(data + (DIM_FIT * DIM_FIT), DIM_FIT);
	gsl_vector *delta = gsl_vector_alloc(DIM_FIT);
	gsl_permutation* permutation = gsl_permutation_alloc(DIM_FIT);

	/* error status of gsl_LU */
	int error;
	
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

	if(p <= PS){
		fprintf(stderr, "Number of process not valid\n");
		MPI_Finalize();
		exit(EXIT_FAILURE);
	}

/*********************************************************************
						EMITTER
*********************************************************************/		
	
	if(my_rank == EMITTER){
		/* THIS IS THE TASK OF THE EMITTER PROCESS */
	#ifdef DEBUG
		printf("EMITTER: RANK %d\n",my_rank);
	#endif		
		/* initialization of the fit */
		initialization(argv[1], input, fit, &matrix, &cropped, &dimx, &dimy, p);
		
		/* send to the workers the parameters and images */
		dim = dimx * dimy;
		for(i = PS; i < p; i++){
			MPI_Send(&dimx, 1, MPI_INT, i, PARAMETERS, MPI_COMM_WORLD);
			MPI_Send(&dimy, 1, MPI_INT, i, PARAMETERS, MPI_COMM_WORLD);
			MPI_Send(fit, DIM_FIT, MPI_DOUBLE, i, RESULTS, MPI_COMM_WORLD);
			MPI_Send(cropped, dim, MPI_UNSIGNED_CHAR, i, IMAGE, MPI_COMM_WORLD);
	#ifdef DEBUG
			printf("EMITTER: DATI INIZIALI INVIATI A %d\n",i);
	#endif
		}
		
#ifdef ON_DEMAND

	#ifdef DEBUG
		printf("ON_DEMAND\n");
	#endif
		j = PS;
		for(i = p - PS; i < STREAMLENGTH; i++){
	#ifdef DEBUG
			printf("EMITTERE IMMAGINE %d\n", i);
	#endif
			flag = 0;
			while ( !flag ){
				if( MPI_Iprobe( j % (p - PS) + PS , MPI_ANY_TAG , MPI_COMM_WORLD , &flag, &status) != MPI_SUCCESS) {
	#ifdef DEBUG
					printf("PROBLEMA NELLA IPROBE DELL'EMITTERE\n");
	#endif
					/*TODO: far schiantare tutto se errore */
				}
				j++;
			}
	#ifdef DEBUG
			printf("MESSAGGIO DA %d \n", j - 1);
	#endif
			/* ricevo la richiesta */
			MPI_Recv(&junk, 1, MPI_INT, (j - 1) % (p - PS) + PS, REQUEST, MPI_COMM_WORLD, &status);	
	#ifdef DEBUG
			printf("REQUEST RICEVUTA\n");
	#endif	
			/*TODO: per ora è la stessa immagine iniziale, eventualmente va cambiata */
			MPI_Send(cropped, dim, MPI_UNSIGNED_CHAR, j % (p - PS) + PS, IMAGE, MPI_COMM_WORLD);
	#ifdef DEBUG
			printf("IMMAGINE MANDATA\n");
	#endif
		}
	
		/* avviso i worker che lo stream è finito */
		for(i = PS; i < p; i++)
			MPI_Send(NULL, 0, MPI_INT, i, TERMINATION, MPI_COMM_WORLD);
	#ifdef DEBUG
		printf("EMITTERE: FINITO DI MANDARE LA TERMINAZIONE...MUORO\n");

	#endif
#else		
		/* funzionamento NON su domanda */	
	
		/* send the cropped image */
		for(i = p - PS; i < STREAMLENGTH; i++){
	#ifdef DEBUG
			printf("EMITTERE IMMAGINE %d\n", i);
	#endif
			MPI_Send(cropped, dim, MPI_UNSIGNED_CHAR, i % (p - PS) + PS, IMAGE, MPI_COMM_WORLD);
	#ifdef DEBUG
			printf("EMITTERE, INVIATA IMMAGINE %d\n", i);
	#endif
		}
#endif				
		

/*********************************************************************
						COLLETTORE
*********************************************************************/

	} else if(my_rank == COLLECTOR){
		/* THIS IS THE TASK OF THE COLLECTOR PROCESS */
	#ifdef DEBUG
		printf("COLLECTOR: RANK %d\n",my_rank);
	#endif
		/* prendo il tempo */
		gettimeofday(&tv1,NULL);

		for(i = 0; i < STREAMLENGTH; i++){

#ifdef ON_DEMAND
			/* se su domanda testo per vedere quale worker ha finito */	
			j = 0;
			flag = 0;
			while ( !flag ){
				MPI_Iprobe( j % (p - PS) + PS , MPI_ANY_TAG , MPI_COMM_WORLD , &flag, &status);
				j++;
			}
			/* e ricevo */
			MPI_Recv(fit, DIM_FIT ,MPI_DOUBLE, (j - 1) % (p - PS) + PS, RESULTS, MPI_COMM_WORLD, &status);

#else
			/* se non su domanda ricevo scandendo in sequenziale */
			MPI_Recv(fit, DIM_FIT ,MPI_DOUBLE, i % (p - PS) + PS, RESULTS, MPI_COMM_WORLD, &status);
#endif		
			
	#ifdef DEBUG
			printf("IMMAGINE %d %f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", i , fit[PAR_A], fit[PAR_X],
			 fit[PAR_Y], fit[PAR_SX], fit[PAR_SY], fit[PAR_a], fit[PAR_b], fit[PAR_c]);
	#endif
		}
		/* prendo il tempo */
		gettimeofday(&tv2,NULL);

		printf("Sono il processo %d (collector), the completion time: %ld\n", 
			my_rank, (tv2.tv_sec - tv1.tv_sec)*1000000 + tv2.tv_usec - tv1.tv_usec);

/*********************************************************************
							WORKER
*********************************************************************/

	} else {
		/* THIS IS THE TASK OF THE WORKER PROCESS */
	#ifdef DEBUG
		printf("WORKER: RANK %d\n",my_rank);
	#endif
		/* receive the dimension of the cropped image */
		MPI_Recv(&dimx, 1, MPI_INT, EMITTER, PARAMETERS, MPI_COMM_WORLD, &status);
		MPI_Recv(&dimy, 1, MPI_INT, EMITTER, PARAMETERS, MPI_COMM_WORLD, &status);
		/* receive the fit of the Gaussian */
		MPI_Recv(fit, DIM_FIT, MPI_DOUBLE, EMITTER, RESULTS, MPI_COMM_WORLD, &status);

	#ifdef DEBUG
		printf("PROCESSO %d, the initial fit: %f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", my_rank, fit[PAR_A], fit[PAR_X],
		fit[PAR_Y], fit[PAR_SX], fit[PAR_SY], fit[PAR_a], fit[PAR_b], fit[PAR_c]);
	#endif
		
		/* define the cropped image */
		dim = dimx * dimy;
		cropped = (unsigned char*) malloc(dim);
#ifdef ON_DEMAND	
		
		while(TRUE){
			/* send the request */
			MPI_Send(&dim, 1, MPI_INT, EMITTER, REQUEST, MPI_COMM_WORLD);
			/* blocking test */
			if(MPI_Probe(EMITTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status) != MPI_SUCCESS){
				/*TODO: se errore schianto tutto */
	#ifdef DEBUG
			printf("ERRORE nel worker durante MPI_Probe\n");
	#endif
			}
			/* se ricevo msg di terminazione esco dal ciclo e muoro */		
			if(status.MPI_TAG == TERMINATION){
	#ifdef DEBUG
				printf("WORKER %d: BECCATO FLAG DI TERMINAZIONE\n",my_rank);
	#endif
				break;
			}	
			MPI_Recv(cropped, dim, MPI_UNSIGNED_CHAR, EMITTER, IMAGE, MPI_COMM_WORLD, &status);
			
			/* image procedure */
			procedure (cropped, dimx, dimy, fit, matrice, vettore);
			gsl_vector_fprintf (stdout, &vettore.vector, "%f");
			gsl_linalg_LU_decomp(&matrice.matrix, permutation, &error); /* TEST ERRORE--> TODO*/
			gsl_linalg_LU_solve(&matrice.matrix, permutation, &vettore.vector, delta);

			for(i = 0; i < DIM_FIT; i++){
				printf("%d: %f + %f =" , i, fit[i], gsl_vector_get(delta, i));
				fit[i]  = fit[i]  + gsl_vector_get(delta, i);
				printf("%f\n", fit[i]);
			}

			MPI_Send(fit, DIM_FIT, MPI_DOUBLE, COLLECTOR , RESULTS, MPI_COMM_WORLD);
		}
	#ifdef DEBUG
		printf("WORKER %d: MUORO\n",my_rank);
	#endif
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
	#ifdef DEBUG
			printf("WORKER %d: RICEVUTA IMMAGINE %d\n",my_rank,i);
	#endif
			/* image procedure */
			procedure (cropped, dimx, dimy, fit, matrice, vettore);

			gsl_linalg_LU_decomp(&matrice.matrix, permutation, &error); /* TEST ERRORE--> TODO*/
			gsl_linalg_LU_solve(&matrice.matrix, permutation, &vettore.vector, delta);

			for(j = 0; j < DIM_FIT; j++){
				printf("%d: %f + %f =" , j, fit[j], gsl_vector_get(delta, j));
				fit[j]  = fit[j]  + gsl_vector_get(delta, j);
				printf("%f\n", fit[j]);
			}

			MPI_Send(fit, DIM_FIT, MPI_DOUBLE, COLLECTOR , RESULTS, MPI_COMM_WORLD);
		}
#endif		
	}
	
	/* Finalize of MPI */
	MPI_Finalize();

	return 0;
}
