#include "fit.h"
#include "parallel.h"
#include "image.h"

/* MPI Variables, global for sake of code simplicity */

int my_rank;			/* MPI rank of the process */
int p = 0;			/* p is the number of processes */

int main(int argc, char *argv[])
{
	
    MPI_Status status;
	
    /* pixels per worker */
    int ppw = 0;
	
	/* pixels of the entire image */
	int dim;
	
    /* dimension of the entire image */
    int width,height;
	
    /* time variables */
    struct timeval tv1, tv2;
	
    /* Dimension of the partitioned image */
    int dimx = 0, dimy = 0;
	
    /* fit of the Gaussian */
    double fit[DIM_FIT];
	
    /* image representing Gaussian fit */
    unsigned char *image = NULL;
	
    /* local partition of the image*/
    unsigned char *partition;
	
    /* indexes */
    int i = 0;
	
	/* buffer for fit procedure */
    double *data = (double *) malloc(sizeof(double) * DIM_FIT * (DIM_FIT + 1));
    gsl_matrix_view matrice = gsl_matrix_view_array(data, DIM_FIT, DIM_FIT);
    gsl_vector_view vettore = gsl_vector_view_array(data + (DIM_FIT * DIM_FIT), DIM_FIT);
	
    /* output buffer of reduce */
    double *ret = (double *) malloc(sizeof(double) * DIM_FIT * (DIM_FIT + 1));
    gsl_matrix_view r_matrice = gsl_matrix_view_array(ret, DIM_FIT, DIM_FIT);
    gsl_vector_view r_vettore = gsl_vector_view_array(ret + (DIM_FIT * DIM_FIT), DIM_FIT);
	
    /* Initialize of MPI */
    MPI_Init(&argc, &argv);
	
	/* check the input parameters */
    if (argc != 3) {
		fprintf(stderr, "Invalid number of parameters: %d\n",argc);
		MPI_Abort(MPI_COMM_WORLD,MPI_ERR_ARG);
    }
    
    /* Every process takes their own rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
    /* Total number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);
	
	/*********************************************************************
	 INIT
	 *********************************************************************/
	
    if (my_rank == EMITTER) {
#if DEBUG
		printf("Emitter with rank %d\n", my_rank);
#endif
		
		/* an image representing the gaussian is created and returned
		 as a unsigned char matrix */
		width = atoi(argv[1]);
		height = atoi(argv[2]);
		/* y dimension of the image is adjusted (if needed) */
		while (height % p != 0) height++;
		/* image is created */
		image = createImage(width, height);
		
		/* parameters of the gaussian are estimated */
		initialization(image, width, height,fit);
		
#if DEBUG
		printf("Emitter sopravvissuto alla init\n");
#endif	
		
		/* send to the workers the parameters and images */
		for (i = PS; i < p; i++) {
			MPI_Send(&width, 1, MPI_INT, i, PARAMETERS, MPI_COMM_WORLD);
			MPI_Send(&height, 1, MPI_INT, i, PARAMETERS, MPI_COMM_WORLD);
			MPI_Send(fit, DIM_FIT, MPI_DOUBLE, i, RESULTS, MPI_COMM_WORLD);
#if DEBUG
			printf("Emitter sends data to %d\n", i);
#endif
		}
    } else {
		
#if DEBUG
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
		
    }
	
    /*********************************************************************
	 LOOP on ELEMENTS
     *********************************************************************/
	
	/* dimension of the local partition are determined and relative buffers
	 are initialized */  
	dim = width * height;
	dimx = width;
	dimy = height / p;
    ppw = dimx * dimy;
    partition = (unsigned char *) malloc(sizeof(unsigned char) * ppw);
    initBuffers(ppw);
	
    /* if I am the emitter I take the time */
    if (my_rank == EMITTER)
		gettimeofday(&tv1, NULL);
	
    for (i = 0; i < STREAMLENGTH; i++) {
		
		/* the emitter executes the scatter */
		MPI_Scatter(image, ppw, MPI_UNSIGNED_CHAR, partition, ppw, MPI_UNSIGNED_CHAR, EMITTER, MPI_COMM_WORLD);
		/* execute the procedure over my partition */
		procedure(partition, dimx, dimy , fit, matrice, vettore);
		
		/* if I am the emitter I execute the reduce */
		MPI_Reduce(data, ret, DIM_FIT * (DIM_FIT + 1), MPI_DOUBLE, MPI_SUM, EMITTER, MPI_COMM_WORLD);
		
		/* and finish the computation */
		if (my_rank == EMITTER) {
			postProcedure(r_matrice,r_vettore,fit);
#ifdef DEBUG
			printf("Image %d: %f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", i, fit[PAR_A], fit[PAR_X],
				   fit[PAR_Y], fit[PAR_SX], fit[PAR_SY], fit[PAR_a], fit[PAR_b], fit[PAR_c]);
#endif
			
		}
		
		/* broadcast of the result */
		MPI_Bcast(fit, DIM_FIT, MPI_DOUBLE, EMITTER, MPI_COMM_WORLD);
		
#ifdef MODULE
		/* new image is stored in buffer image */
#endif			
    }
	
    if (my_rank == EMITTER) {
		gettimeofday(&tv2, NULL);
		/* print the parallelism degree, data size and the completion time */
		printf("%d\t%d\t%ld\n", p, dim,  (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec);
    }
    
    /* Finalize of MPI */
    MPI_Finalize();
    
    return 0;
}
