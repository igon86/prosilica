#include "fit.h"
#include "parallel.h"

int my_rank;

int main(int argc, char* argv[]){
	
	/* MPI Variables */
	int p = 0; /* p is the number of processes */
	MPI_Status status;
	
	/* pixels per worker */
	int ppw = 0;

	/* time variables */
	struct timeval tv1, tv2;

	/* Dimension of the cropped image */
	int dimx = 0, dimy = 0;
	
	/* fit of the Gaussian */
	double fit [DIM_FIT];
	
	/* image representing Gaussian fit */	
	unsigned char *matrix = NULL;
	
	/* cropped image taken from a Gaussian image */
	unsigned char *cropped = NULL;
	
	/* local partition */
	unsigned char* partition;

	/* indexes */
	int i = 0, j = 0;
	
	/* data for the LU solver */
	gsl_vector *delta = gsl_vector_alloc(DIM_FIT);
	gsl_permutation* permutation = gsl_permutation_alloc(DIM_FIT);
	
	/* error status of gsl_LU */
	int error = 0;

	double* data = (double*) malloc(sizeof(double) * DIM_FIT * (DIM_FIT + 1));
	gsl_matrix_view matrice = gsl_matrix_view_array(data, DIM_FIT, DIM_FIT);
	gsl_vector_view vettore = gsl_vector_view_array(data + (DIM_FIT * DIM_FIT), DIM_FIT);

	/* output buffer of reduce */
	double* ret = (double*) malloc(sizeof(double) * DIM_FIT * (DIM_FIT + 1));
	gsl_matrix_view r_matrice = gsl_matrix_view_array(ret, DIM_FIT, DIM_FIT);
	gsl_vector_view r_vettore = gsl_vector_view_array(ret + (DIM_FIT * DIM_FIT), DIM_FIT);
	
#ifdef DEBUG
	/* file per la scrittura dei risultati 
	FILE* risultati;
	risultati = fopen("dp_result","w");
	*/
#endif

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
	/* Every process takes their own rank*/
	if(MPI_Comm_rank (MPI_COMM_WORLD, &my_rank) != MPI_SUCCESS){
		fprintf(stderr, "MPI_Comm_rank failed\n");
		exit(EXIT_FAILURE);
	}
	/* Total number of processes */
	if(MPI_Comm_size (MPI_COMM_WORLD, &p) != MPI_SUCCESS){
		fprintf(stderr, "MPI_Comm_size failed\n");
		exit(EXIT_FAILURE);
	}

/*********************************************************************
						INIT
*********************************************************************/		

	if(my_rank == EMITTER){
#if DEBUG
		printf("Emitter with rank %d\n", my_rank);
#endif		
		/* initialization of the fit */
		initialization(argv[1], fit, &matrix, &cropped, &dimx, &dimy, p);
	
		/* send to the workers the parameters and images */
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
#if DEBUG
			printf("Emitter sends data to %d\n",i);
#endif
		}
	} else { 
#if DEBUG
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

	}
	
	
	/*********************************************************************
						LOOP on ELEMENTS
	 *********************************************************************/		
	
	ppw = (dimx * dimy) / p;
	partition = (unsigned char*) malloc(sizeof(unsigned char) * ppw);

	/* if I am the emitter I take the time */
	if(my_rank == EMITTER)
		gettimeofday(&tv1, NULL);

	for (i = 0; i < STREAMLENGTH; i++){
		
		/* the emitter executes the scatter */
		if(MPI_Scatter (cropped, ppw, MPI_UNSIGNED_CHAR, partition, ppw, MPI_UNSIGNED_CHAR, EMITTER, MPI_COMM_WORLD) != MPI_SUCCESS){
			fprintf(stderr, "MPI_Scatter failed\n");
			exit(EXIT_FAILURE);
		}
						 
		/* execute the procedure over my partition */
		procedure (partition, dimx, dimy/p, fit, matrice, vettore);
		
		/* if I am the emitter I execute the reduce */
		if(MPI_Reduce (data, ret, DIM_FIT * (DIM_FIT + 1), MPI_DOUBLE, MPI_SUM, EMITTER, MPI_COMM_WORLD) != MPI_SUCCESS){
			fprintf(stderr, "MPI_Reduce failed\n");
			exit(EXIT_FAILURE);
		}	
		
		/* and finish the computation */
		if (my_rank == EMITTER){
			gsl_linalg_LU_decomp(&r_matrice.matrix, permutation, &error); /* TEST ERRORE--> TODO*/
			gsl_linalg_LU_solve(&r_matrice.matrix, permutation, &r_vettore.vector, delta);
			
			for(j = 0; j < DIM_FIT; j++)
				fit[j]  = fit[j]  + gsl_vector_get(delta, j);
#ifdef DEBUG
		printf("Image %d: %f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", i , fit[PAR_A], fit[PAR_X], 
			fit[PAR_Y], fit[PAR_SX], fit[PAR_SY], fit[PAR_a], fit[PAR_b], fit[PAR_c]);
#endif
		}
		
		/* broadcast of the result */
		MPI_Bcast ( fit, DIM_FIT, MPI_DOUBLE, EMITTER, MPI_COMM_WORLD );
	}
	
	if(my_rank == EMITTER){
		gettimeofday(&tv2, NULL);
		printf("Process with rank %d (emitter), the completion time: %ld\n", 
			my_rank, (tv2.tv_sec - tv1.tv_sec)*1000000 + tv2.tv_usec - tv1.tv_usec);
	}
	
	/* Finalize of MPI */
	if(MPI_Finalize() != MPI_SUCCESS){
		fprintf(stderr, "MPI_Finalize failed\n");
		exit(EXIT_FAILURE);
	}
	
	return 0;
}
