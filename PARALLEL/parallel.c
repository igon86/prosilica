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
	
	/* fits of Gaussian */
	double fit [DIM_FIT];
	/* image representing Gaussian fit */	
	unsigned char *matrix = NULL;
	/* cropped image taken from a Gaussian image */
	unsigned char *cropped = NULL;

	/* indexes */
	int i = 0, j = 0;

	/* Gauss matrix and vector have contiguous space in memory */
	double* data = (double*) malloc(sizeof(double) * DIM_FIT * (DIM_FIT + 1));
	gsl_matrix_view matrice = gsl_matrix_view_array(data, DIM_FIT, DIM_FIT);
	gsl_vector_view vettore = gsl_vector_view_array(data + (DIM_FIT * DIM_FIT), DIM_FIT);
	/* vector of solution */
	gsl_vector *delta = gsl_vector_alloc(DIM_FIT);
	/* data for the LU solver */
	gsl_permutation* permutation = gsl_permutation_alloc(DIM_FIT);

	/* error status of gsl_LU */
	int error;
	
	/* check the input parameters */
	if(argc != 2){
		fprintf(stderr, "Invalid number of parameters\n");
		exit(EXIT_FAILURE);
	}

	/* Initialize of MPI */
	if(MPI_Init (&argc, &argv) != MPI_SUCCESS){
		fprintf(stderr, "MPI_Init failed\n");
		exit(EXIT_FAILURE);
	}
	/* Every process takes the own rank*/
	if(MPI_Comm_rank (MPI_COMM_WORLD, &my_rank) != MPI_SUCCESS){
		fprintf(stderr, "MPI_Comm_rank failed\n");
		exit(EXIT_FAILURE);
	}
	/* Total number of processes */
	if(MPI_Comm_size (MPI_COMM_WORLD, &p) != MPI_SUCCESS){
		fprintf(stderr, "MPI_Comm_size failed\n");
		exit(EXIT_FAILURE);
	}

	if(p <= PS){
		fprintf(stderr, "Number of process not valid\n");
		exit(EXIT_FAILURE);
	}

/*********************************************************************
						EMITTER
*********************************************************************/		
	
	if(my_rank == EMITTER){
	#ifdef DEBUG
		printf("Emitter with rank %d\n", my_rank);
	#endif		
		/* initialization of the fit */
		initialization(argv[1], fit, &matrix, &cropped, &dimx, &dimy, p);
		
		/* send to the workers the parameters and images */
		dim = dimx * dimy;
		for(i = PS; i < p; i++){
			if(MPI_Send(&dimx, 1, MPI_INT, i, PARAMETERS, MPI_COMM_WORLD) != MPI_SUCCESS){
				fprintf(stderr, "MPI_Send failed\n");
				exit(EXIT_FAILURE);
			}
			if(MPI_Send(&dimy, 1, MPI_INT, i, PARAMETERS, MPI_COMM_WORLD) != MPI_SUCCESS){
				fprintf(stderr, "MPI_Send failed\n");
				exit(EXIT_FAILURE);
			}
			if(MPI_Send(fit, DIM_FIT, MPI_DOUBLE, i, RESULTS, MPI_COMM_WORLD) != MPI_SUCCESS){
				fprintf(stderr, "MPI_Send failed\n");
				exit(EXIT_FAILURE);
			}
			if(MPI_Send(cropped, dim, MPI_UNSIGNED_CHAR, i, IMAGE, MPI_COMM_WORLD) != MPI_SUCCESS){
				fprintf(stderr, "MPI_Send failed\n");
				exit(EXIT_FAILURE);
			}
	#ifdef DEBUG
			printf("Emitter sends data to %d\n", i);
	#endif
		}
		
#ifdef ON_DEMAND
		j = PS;
		for(i = p - PS; i < STREAMLENGTH; i++){
	#ifdef DEBUG
			printf("Emitter sends image %d\n", i);
	#endif
			flag = 0;
			while ( !flag ){
				if( MPI_Iprobe( j % (p - PS) + PS , MPI_ANY_TAG , MPI_COMM_WORLD , &flag, &status) != MPI_SUCCESS) {
					fprintf(stderr, "MPI_Iprobe failed\n");
					exit(EXIT_FAILURE);
				}
				j++;
			}

			/* receive the request */
			if(MPI_Recv(&junk, 1, MPI_INT, (j - 1) % (p - PS) + PS, REQUEST, MPI_COMM_WORLD, &status) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Recv failed\n");
				exit(EXIT_FAILURE);
			}
			/* send the image */
			if(MPI_Send(cropped, dim, MPI_UNSIGNED_CHAR, j % (p - PS) + PS, IMAGE, MPI_COMM_WORLD) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Send failed\n");
				exit(EXIT_FAILURE);
			}
		}
	
		/* send the termination message */
		for(i = PS; i < p; i++) {
			if(MPI_Send(NULL, 0, MPI_INT, i, TERMINATION, MPI_COMM_WORLD) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Iprobe failed\n");
				exit(EXIT_FAILURE);
			}
		}
#else		
		/* NOT ON_DEMAND */	
	
		/* send the cropped image */
		for(i = p - PS; i < STREAMLENGTH; i++){
	#ifdef DEBUG
			printf("Emitter sends image %d\n", i);
	#endif
			if(MPI_Send(cropped, dim, MPI_UNSIGNED_CHAR, i % (p - PS) + PS, IMAGE, MPI_COMM_WORLD) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Send failed\n");
				exit(EXIT_FAILURE);
			}
		}
#endif				
		

/*********************************************************************
						COLLECTOR
*********************************************************************/

	} else if(my_rank == COLLECTOR){
	#ifdef DEBUG
		printf("Collector with rank %d\n", my_rank);
	#endif
		/* take the time */
		gettimeofday(&tv1, NULL);

		for(i = 0; i < STREAMLENGTH; i++){

#ifdef ON_DEMAND
			/* the collector tests for know which worker has terminated */	
			j = 0;
			flag = 0;
			while ( !flag ){
				if(MPI_Iprobe( j % (p - PS) + PS , MPI_ANY_TAG , MPI_COMM_WORLD , &flag, &status) != MPI_SUCCESS) {
					fprintf(stderr, "MPI_Iprobe failed\n");
					exit(EXIT_FAILURE);
				}
				j++;
			}
			/* and receive */
			if(MPI_Recv(fit, DIM_FIT ,MPI_DOUBLE, (j - 1) % (p - PS) + PS, RESULTS, MPI_COMM_WORLD, &status) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Recv failed\n");
				exit(EXIT_FAILURE);
			}

#else
			/* if NOT ON_DEMAND the colector receives linearly */
			if(MPI_Recv(fit, DIM_FIT ,MPI_DOUBLE, i % (p - PS) + PS, RESULTS, MPI_COMM_WORLD, &status) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_recv failed\n");
				exit(EXIT_FAILURE);
			}
#endif		
			
#ifdef DEBUG
			/* PRINTOUT OF THE CURRENT RESULT */
			printf("Image %d: %f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", i , fit[PAR_A], fit[PAR_X], 
				fit[PAR_Y], fit[PAR_SX], fit[PAR_SY], fit[PAR_a], fit[PAR_b], fit[PAR_c]);
#endif
		}
		/* take the time */
		gettimeofday(&tv2, NULL);

		printf("Process with rank %d (collector), the completion time: %ld\n", 
			my_rank, (tv2.tv_sec - tv1.tv_sec)*1000000 + tv2.tv_usec - tv1.tv_usec);

/*********************************************************************
							WORKER
*********************************************************************/

	} else {
	#ifdef DEBUG
		printf("Worker with rank %d\n", my_rank);
	#endif
		/* receive the dimension of the cropped image */
		if(MPI_Recv(&dimx, 1, MPI_INT, EMITTER, PARAMETERS, MPI_COMM_WORLD, &status) != MPI_SUCCESS) {
			fprintf(stderr, "MPI_recv failed\n");
			exit(EXIT_FAILURE);
		}
		if(MPI_Recv(&dimy, 1, MPI_INT, EMITTER, PARAMETERS, MPI_COMM_WORLD, &status) != MPI_SUCCESS) {
			fprintf(stderr, "MPI_recv failed\n");
			exit(EXIT_FAILURE);
		}
		/* receive the fit of the Gaussian */
		if(MPI_Recv(fit, DIM_FIT, MPI_DOUBLE, EMITTER, RESULTS, MPI_COMM_WORLD, &status) != MPI_SUCCESS) {
			fprintf(stderr, "MPI_recv failed\n");
			exit(EXIT_FAILURE);
		}

	#ifdef DEBUG
			printf("Process %d, the initial fit is:\n%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", my_rank, fit[PAR_A], fit[PAR_X],
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
				fprintf(stderr, "MPI_Iprobe failed\n");
				exit(EXIT_FAILURE);
			}

			/* workers ends if receives termination message */		
			if(status.MPI_TAG == TERMINATION){
	#ifdef DEBUG
				printf("Worker %d ends\n", my_rank);
	#endif
				break;
			}
	
			if(MPI_Recv(cropped, dim, MPI_UNSIGNED_CHAR, EMITTER, IMAGE, MPI_COMM_WORLD, &status) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_recv failed\n");
				exit(EXIT_FAILURE);
			}
			
			/* calculation of the Gauss matrix and vector */
			procedure (cropped, dimx, dimy, fit, matrice, vettore);
			gsl_linalg_LU_decomp(&matrice.matrix, permutation, &error); // TEST ERRORE--> TODO
			gsl_linalg_LU_solve(&matrice.matrix, permutation, &vettore.vector, delta);

			for(i = 0; i < DIM_FIT; i++)
				fit[i]  = fit[i]  + gsl_vector_get(delta, i);

			if(MPI_Send(fit, DIM_FIT, MPI_DOUBLE, COLLECTOR , RESULTS, MPI_COMM_WORLD) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Send failed\n");
				exit(EXIT_FAILURE);
			}
		}
		
#else	/* NOT ON_DEMAND */
		
		/* receive the number of the images */
		num_image = STREAMLENGTH / (p - PS);		
		if(STREAMLENGTH % (p - PS) > (my_rank - PS))
			num_image++;

		/* work on the images and send them to the collector */
		for (i = 0; i < num_image ; i++){
			if(MPI_Recv(cropped, dim, MPI_UNSIGNED_CHAR, EMITTER, IMAGE, MPI_COMM_WORLD, &status) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Recv failed\n");
				exit(EXIT_FAILURE);
			}

			/* image procedure */
			procedure (cropped, dimx, dimy, fit, matrice, vettore);

			gsl_linalg_LU_decomp(&matrice.matrix, permutation, &error); /* TEST ERRORE--> TODO*/
			gsl_linalg_LU_solve(&matrice.matrix, permutation, &vettore.vector, delta);

			for(j = 0; j < DIM_FIT; j++)
				fit[j]  = fit[j]  + gsl_vector_get(delta, j);

			if(MPI_Send(fit, DIM_FIT, MPI_DOUBLE, COLLECTOR , RESULTS, MPI_COMM_WORLD) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Send failed\n");
				exit(EXIT_FAILURE);
			}
		}
#endif		
	}
	
	/* Finalize of MPI */
	if(MPI_Finalize() != MPI_SUCCESS){
		fprintf(stderr, "MPI_Finalize failed\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}
