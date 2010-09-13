#include "fit.h"
#include "parallel.h"
#include "image.h"

/* MPI Variables, global for sake of code simplicity */

int my_rank;			/* MPI rank of the process		 */
int p = 0;				/* p is the number of processes */

int main(int argc, char *argv[])
{
	
#ifdef ON_DEMAND
    int junk = 0;
#endif
	/* return status of MPI functions */
    MPI_Status status;
    /* number of pixels of the image */
    int dim = 0;
    /* dimensions of the image */
    int width,height;
	
    /* time variables */
    struct timeval tv1, tv2;
	
    /* fits of Gaussian */
    double fit[DIM_FIT];
    /* image representing Gaussian fit */
    unsigned char* image = NULL;
	
	/* indexes */
    int i = 0;
	
#ifdef MODULO
	int w = 0;
#endif
	
    /* Gauss matrix and vector have contiguous space in memory */
    double *data = (double *) malloc(sizeof(double) * DIM_FIT * (DIM_FIT + 1));
    gsl_matrix_view matrice = gsl_matrix_view_array(data, DIM_FIT, DIM_FIT);
    gsl_vector_view vettore = gsl_vector_view_array(data + (DIM_FIT * DIM_FIT), DIM_FIT);
	
	/* random number generator seed */
	srand(time(NULL));
	
    /* Initialize of MPI */
    MPI_Init(&argc, &argv);
	
    /* check the input parameters */
    if (argc != 3) {
		fprintf(stderr, "Invalid number of parameters\n");
		MPI_Abort(MPI_COMM_WORLD,MPI_ERR_ARG);
    }
	
    /* Every process takes the own rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
    /* Total number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);
	
	/* check for the number of processes */
    if (p <= PS) {
		fprintf(stderr, "Number of process not valid\n");
		MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OP);
    }
	
	/*********************************************************************
	 EMITTER
	 *********************************************************************/
	
    if (my_rank == EMITTER) {
		
#ifdef DEBUG
		printf("Emitter with rank %d\n", my_rank);
#endif
		
#ifdef MODULE  /* --------------------------------- NEW */	
		
#ifdef ON_DEMAND
		j = PS;
#endif
		for(i = 0; i < STREAMLENGTH; i++){
			/* receive the image --> TODO, now there is createImage and initialization above */
			
			/* send to collector and workers the parameters */
			if(i == 0){
				MPI_Send(&dim, 1, MPI_INT, COLLECTOR, PARAMETERS, MPI_COMM_WORLD);
				for(w = PS; w < p; w++){
					MPI_Send(&width, 1, MPI_INT, w, PARAMETERS, MPI_COMM_WORLD);
					MPI_Send(&height, 1, MPI_INT, w, PARAMETERS, MPI_COMM_WORLD);
					MPI_Send(fit, DIM_FIT, MPI_DOUBLE, w, RESULTS, MPI_COMM_WORLD);
				}
			}
			/* send the image */
#ifdef ON_DEMAND
	    	flag = 0;
	    	while (!flag) {
				MPI_Iprobe(j % (p - PS) + PS, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
				j++;
	    	}
			
			/* receive the request */
	    	MPI_Recv(&junk, 1, MPI_INT, (j - 1) % (p - PS) + PS, REQUEST, MPI_COMM_WORLD, &status);
			
	    	/* send the image */
	    	MPI_Send(image, dim, MPI_UNSIGNED_CHAR, j % (p - PS) + PS, IMAGE, MPI_COMM_WORLD);
#else
			MPI_Send(image, dim, MPI_UNSIGNED_CHAR, i % (p - PS) + PS, IMAGE, MPI_COMM_WORLD);
#endif
		}
		
#ifdef ON_DEMAND
		/* send the termination message */
		for (i = PS; i < p; i++)
			MPI_Send(NULL, 0, MPI_INT, i, TERMINATION, MPI_COMM_WORLD);
#endif
		
#else  /* --------------------------------- END NEW */
		
		/* an image representing the gaussian is created and returned
		 as a unsigned char matrix */
		width = atoi(argv[1]);
		height = atoi(argv[2]);
		image = createImage(width, height);
		
		/* parameters of the gaussian are estimated */
		initialization(image, width, height,fit);
		
		dim = width * height;
		
		/* send to collector the number of pixels */
		MPI_Send(&dim, 1, MPI_INT, COLLECTOR, PARAMETERS, MPI_COMM_WORLD);
		
		/* send to the workers the parameters and images */	
		for (i = PS; i < p; i++) {
			MPI_Send(&width, 1, MPI_INT, i, PARAMETERS, MPI_COMM_WORLD);
			MPI_Send(&height, 1, MPI_INT, i, PARAMETERS, MPI_COMM_WORLD);
			MPI_Send(fit, DIM_FIT, MPI_DOUBLE, i, RESULTS, MPI_COMM_WORLD);
			/*	first image is sent */
#ifdef MODULE
			/* image = metodo_da_fare */
#endif				
			MPI_Send(image, dim, MPI_UNSIGNED_CHAR, i, IMAGE, MPI_COMM_WORLD);
		}
		
		for (i = p - PS; i < STREAMLENGTH; i++) {
			
#ifdef DEBUG
			printf("Emitter sends image %d\n", i);
#endif
			
#ifdef ON_DEMAND  /* ON_DEMAND */
			
			/* receive the request */
			MPI_Recv(&junk, 1, MPI_INT, MPI_ANY_SOURCE, REQUEST, MPI_COMM_WORLD, &status);
			
			/* send the image */
			MPI_Send(image, dim, MPI_UNSIGNED_CHAR, status.MPI_SOURCE, IMAGE, MPI_COMM_WORLD);
			
#else		/* NOT ON_DEMAND */
			MPI_Send(image, dim, MPI_UNSIGNED_CHAR, i % (p - PS) + PS, IMAGE, MPI_COMM_WORLD);
#endif			
		}
		
		/* send the termination message */
		for (i = PS; i < p; i++)
			MPI_Send(NULL, 0, MPI_INT, i, TERMINATION, MPI_COMM_WORLD);
		
		/* send the image */
		for (i = p - PS; i < STREAMLENGTH; i++) {
#ifdef DEBUG
			printf("Emitter sends image %d\n", i);
#endif
			
		}
		
#endif
		
		/*********************************************************************
		 COLLECTOR
		 *********************************************************************/
		
    } else if (my_rank == COLLECTOR) {
#ifdef DEBUG
		printf("Collector with rank %d\n", my_rank);
#endif
		MPI_Recv(&dim, 1, MPI_INT, EMITTER, PARAMETERS, MPI_COMM_WORLD, &status);
		/* take the time */
		gettimeofday(&tv1, NULL);
		i = 0;
		while (i < p - PS) {
			
			MPI_Recv(fit, DIM_FIT, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			if (status.MPI_TAG == TERMINATION) {
#ifdef DEBUG
				printf("Worker %d has ended\n", status.MPI_SOURCE);
#endif
				i++;
			}	
#ifdef DEBUG
			/* PRINTOUT OF THE CURRENT RESULT */
			printf("Image %d: %f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", i, fit[PAR_A], fit[PAR_X],
				   fit[PAR_Y], fit[PAR_SX], fit[PAR_SY], fit[PAR_a], fit[PAR_b], fit[PAR_c]);
#endif
		}
		/* take the time */
		gettimeofday(&tv2, NULL);
		
		/* print parallelism degree, data size and the completion time */
		printf("%d\t%d\t%ld\n", p, dim, (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec);
		
		/*********************************************************************
		 WORKER
		 *********************************************************************/
		
    } else {
#ifdef DEBUG
		printf("Worker with rank %d\n", my_rank);
#endif
		/* receive the dimension of the image */
		MPI_Recv(&width, 1, MPI_INT, EMITTER, PARAMETERS, MPI_COMM_WORLD, &status);
		MPI_Recv(&height, 1, MPI_INT, EMITTER, PARAMETERS, MPI_COMM_WORLD, &status);
		/* receive the fit of the Gaussian */
		MPI_Recv(fit, DIM_FIT, MPI_DOUBLE, EMITTER, RESULTS, MPI_COMM_WORLD, &status);
#ifdef DEBUG
		printf("Process %d, the initial fit is:\n%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", my_rank, fit[PAR_A], fit[PAR_X],
			   fit[PAR_Y], fit[PAR_SX], fit[PAR_SY], fit[PAR_a], fit[PAR_b], fit[PAR_c]);
#endif
		
		/* calculate number of pixels and initialize buffers for the fit */
		dim = width * height;
		initBuffers(dim);
		image = (unsigned char *) malloc(dim);
		
		while (TRUE) {
#ifdef ON_DEMAND		
			/* send the request */
			MPI_Send(&dim, 1, MPI_INT, EMITTER, REQUEST, MPI_COMM_WORLD);
#endif			
			/* blocking test of incoming message */
			MPI_Probe(EMITTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			/* workers ends if receives termination message */
			if (status.MPI_TAG == TERMINATION) {
#ifdef DEBUG
				printf("Worker %d ends\n", my_rank);
#endif
				/* last result is sent to the collector with a different TAG */
				MPI_Send(fit, DIM_FIT, MPI_DOUBLE, COLLECTOR, TERMINATION, MPI_COMM_WORLD);
				break;
			}
			/* receive the image from the emitter */
			MPI_Recv(image, dim, MPI_UNSIGNED_CHAR, EMITTER, IMAGE, MPI_COMM_WORLD, &status);
			/* calculation of the Gauss matrix and vector */
			procedure(image, width, height, fit, matrice, vettore);
			/* solve the system and adjust the fit vector */
			postProcedure(matrice,vettore,fit);
			/* send the result to the collector */
			MPI_Send(fit, DIM_FIT, MPI_DOUBLE, COLLECTOR, RESULTS, MPI_COMM_WORLD);
		}
    }
	
    /* Finalize of MPI */
    MPI_Finalize();
	
    return 0;
}
