#include "fit.h"
#include "scatter.h"
#include "parallel.h"
#include "image.h"

/* MPI Variables, global for sake of code simplicity */

int my_rank;			/* MPI rank of the process */
int p = 0;			/* p is the number of processes */

int main(int argc, char *argv[])
{
	
    /* pixels per worker */
    int ppw = 0;
	
	/* number of workers */
	int num_worker;
	
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
	
	/* size of reduce buffer */
	int buffer_size = DIM_FIT * (DIM_FIT + 1);
	
    /* indexes */
    int i = 0;
	
	/* buffer for fit procedure */
    double *data = (double *) malloc( sizeof(double) * buffer_size );
    gsl_matrix_view matrice = gsl_matrix_view_array(data, DIM_FIT, DIM_FIT);
    gsl_vector_view vettore = gsl_vector_view_array(data + (DIM_FIT * DIM_FIT), DIM_FIT);
	
    /* output buffer of reduce */
    double *ret = (double *) malloc( sizeof(double) * buffer_size );
    gsl_matrix_view r_matrice = gsl_matrix_view_array(ret, DIM_FIT, DIM_FIT);
    gsl_vector_view r_vettore = gsl_vector_view_array(ret + (DIM_FIT * DIM_FIT), DIM_FIT);
	
#ifdef MODULE
	/* socket for the camera */
	int new_sock;
#endif	
	
	/*********************************************************************
	 INIT
	 *********************************************************************/	
	 
	srand(time(NULL));
	
    /* Initialize of MPI */
    MPI_Init(&argc, &argv);
	
	/* Check the input parameters */
    if (argc != 3) {
		fprintf(stderr, "Invalid number of parameters: %d\n",argc);
		MPI_Abort(MPI_COMM_WORLD,MPI_ERR_ARG);
    }
    
    /* Every process takes their own rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
    /* Total number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);
	
	/* Number of workers */ 
	num_worker = p - PS;
	
    if (my_rank == EMITTER) {
#if DEBUG
		printf("Emitter with rank %d\n", my_rank);
#endif	

#ifndef MODULE
		/* an image representing the gaussian is created and returned
		 as a unsigned char matrix */
		width = atoi(argv[1]);
		height = atoi(argv[2]);
		/* y dimension of the image is adjusted (if needed) */
		while (height % num_worker != 0) height++;
		/* image is created */
		image = createImage(width, height);
#else
		new_sock = Connect();
		
		if (Read(new_sock, &width, sizeof(int)) < 0)
			error("Error reading the integers");
		if (Read(new_sock, &height, sizeof(int)) < 0)
			error("Error reading the integers");
		
		dim = width * height;
		image = (unsigned char *) malloc (dim);
		/* read from the socket */
		if (Read(new_sock, image, dim) < 0)
			error("Error reading from socket");
#endif				
		/* parameters of the gaussian are estimated */
		initialization(image, width, height,fit);
				
    } 
	
	/* broadcast data of the image */
	MPI_Bcast(&width,1,MPI_INT,EMITTER,MPI_COMM_WORLD);
	MPI_Bcast(&height,1,MPI_INT,EMITTER,MPI_COMM_WORLD);
	MPI_Bcast(&fit,DIM_FIT,MPI_DOUBLE,EMITTER,MPI_COMM_WORLD);
	
	/* dimension of the local partition are determined and relative buffers
	 are initialized */  
	
	dim = width * height;
	dimx = width;
	dimy = height / num_worker;
    ppw = dimx * dimy;
    partition = (unsigned char *) malloc(sizeof(unsigned char) * ppw);
    initBuffers(ppw);
	
    /* if I am the emitter I take the time */
    if (my_rank == EMITTER)
		gettimeofday(&tv1, NULL);
	
    /*********************************************************************
	 LOOP on ELEMENTS
     *********************************************************************/		
	
    for (i = 0; i < STREAMLENGTH; i++) {
		
		/* the emitter executes the scatter */
		scatterImage(image, dim, partition);
		
		/* execute the procedure over my partition */		
		if(my_rank >= PS){
			procedure(partition, dimx, dimy , fit, matrice, vettore, dimy*(my_rank-PS));	
		}
				
		/* gather/reduce is executed */
		reduceMatrix(ret, buffer_size, data);
		
		if (my_rank == COLLECTOR) {
			/* adjust fit results */
			postProcedure(r_matrice,r_vettore,fit);
#if DEBUG			
			printf("Image %d: %f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", i, fit[PAR_A], fit[PAR_X],
				   fit[PAR_Y], fit[PAR_SX], fit[PAR_SY], fit[PAR_a], fit[PAR_b], fit[PAR_c]);
#endif				   
		}
		
		/* broadcast of the result */
		MPI_Bcast(fit, DIM_FIT, MPI_DOUBLE, COLLECTOR, MPI_COMM_WORLD);
		
#ifdef MODULE
		if (my_rank == EMITTER && i< STREAMLENGTH - 1) {
			/* read from the socket */
		if (Read(new_sock, image, dim) < 0)
			error("Error reading from socket");

		}
#endif			
    }
	
    if (my_rank == EMITTER) {
		gettimeofday(&tv2, NULL);
		/* print the parallelism degree, data size and the completion time */
		printf("%d\t%d\t%ld\n", p, dim,  (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec);
    }
    
	freeBuffers();
    /* Finalize of MPI */
    MPI_Finalize();
    
    return 0;
}
