#include <stdio.h>
#include <mpi.h>
#include <sys/time.h>

#include "tiffPostElaboration.hpp"
#include "fit.hpp"

#define OUTPUT_MATRIX "gaussiana.tiff" 
#define CROP_PARAMETER 0.5
#define STREAMLENGTH 100

#define EMETTITORE 0
#define COLLETTORE 1
#define PS 2

#define PARAMETERS 0
#define IMAGE 1
#define RESULTS 2

extern FILE *risultati;

int main(int argc, char* argv[]){
	
	FILE* parameters;
	
	/* MPI VARIABLES */
	int my_rank,p;
	MPI_Status status;
	
	/* parameters of the gaussian */
	int width;
	int length;
	/* DA CAMBIARE CON UN MPI_BLOCK */
	double amplitude;
	double x_0;
	double y_0;
	double sigma_x0;
	double sigma_y0;
	double a_0;
	double b_0;
	double c_0;
	int max,min;
	int dim;
	int num_image;

	/* time variables */
	struct timeval tv1,tv2;
	
	/* parameters for the cookie cutter */
	double x0,y0;
	double FWHM_x,FWHM_y;
	int span_x,span_y;
	int dimx,dimy;
	int x,y;
	
	/* gaussian struct */
	//	fit_t results,test_g;
	double result [DIM_FIT], fit [DIM_FIT];	

	unsigned char *cropped;
	
	/* indexes */
	int i,j;
	int temp;

	if(argc != 2){
		printf("NUMERO PARAMETRI INVALIDO\n");
		exit(EXIT_FAILURE);
	}

	MPI_Init (&argc,&argv);
	
	MPI_Comm_rank (MPI_COMM_WORLD, &my_rank);

	MPI_Comm_size (MPI_COMM_WORLD, &p);

	if(my_rank == EMETTITORE){

		parameters = fopen(argv[1],"r");
	
		/* LETTURA DEI PARAMETRI */
		fscanf(parameters,"%d\t%d\t", &width, &length);
		for(i = 0; i < DIM_FIT; i++){
			fscanf(parameters,"%lf\t",&result[i]);		
			fit[i]=0;
		}



		/* GAUSSIAN_MATRIX */
		unsigned char matrix[length][width];
	
		/* COSTRUZIONE DELL'IMMAGINE */
		for (i=0;i<length;i++){
			for(j=0;j<width;j++){
				temp = (int) evaluateGaussian(result,j,i);
				//printf("%d\n",temp);
				matrix[i][j] = temp;
			}
		}
	
		/* WRITING THE IMAGE TO BE FITTED ON A TIFF FILE */
#if DEBUG
		writeImage((unsigned char *)matrix,(char *) OUTPUT_MATRIX, width, length);
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
		 inizialization of the test_g struct.
		 NOTE: the coordinates of the position (x,y) are relative to the cropped portion of the image.
		 the value of span_x, which is approximately the diameter of the gaussian, is generally not as bad
		 as you may think to start the fit. 
		 */
		fit[PAR_A] = max;
		fit[PAR_X] = span_x;
		fit[PAR_Y] = span_y;
		fit[PAR_SX] = FWHM_x;
		fit[PAR_SY] = FWHM_y;
		fit[PAR_c] = min;	
	
		/* THIS PART CAN BE ITERATIVE */
	
		unsigned char *cropped = cropImage((unsigned char*) matrix, width, length, x - span_x, x + span_x, y - span_y, y + span_y);
	
#if DEBUG
		writeImage(cropped, (char *) "./CROP.tiff", dimx, dimy);
#endif
		// invio al worker i differenti parametri
		dim=dimx*dimy;
		for(i=PS;i<p;i++){
			MPI_Send(&dimx, 1, MPI_INT, i, PARAMETERS, MPI_COMM_WORLD);
			MPI_Send(&dimy, 1, MPI_INT, i, PARAMETERS, MPI_COMM_WORLD);
			MPI_Send(fit, DIM_FIT, MPI_DOUBLE, i, PARAMETERS, MPI_COMM_WORLD);
		}
		
		for(i=0; i < STREAMLENGTH; i++){

			MPI_Send(cropped, dim, MPI_BYTE, i%(p-2)+2, IMAGE, MPI_COMM_WORLD);

		}
	}
	else if(my_rank == COLLETTORE){
		gettimeofday(&tv1,NULL);
		for(i=0; i < STREAMLENGTH; i++){

			MPI_Recv(&test_g.A,1,MPI_DOUBLE,i%(p-2)+2,RESULTS,MPI_COMM_WORLD,&status);
			MPI_Recv(&test_g.x_0,1,MPI_DOUBLE,i%(p-2)+2,RESULTS,MPI_COMM_WORLD,&status);
			MPI_Recv(&test_g.y_0,1,MPI_DOUBLE,i%(p-2)+2,RESULTS,MPI_COMM_WORLD,&status);
			MPI_Recv(&test_g.sigma_x,1,MPI_DOUBLE,i%(p-2)+2,RESULTS,MPI_COMM_WORLD,&status);
			MPI_Recv(&test_g.sigma_y,1,MPI_DOUBLE,i%(p-2)+2,RESULTS,MPI_COMM_WORLD,&status);
			MPI_Recv(&test_g.a,1,MPI_DOUBLE,i%(p-2)+2,RESULTS,MPI_COMM_WORLD,&status);
			MPI_Recv(&test_g.b,1,MPI_DOUBLE,i%(p-2)+2,RESULTS,MPI_COMM_WORLD,&status);
			MPI_Recv(&test_g.c,1,MPI_DOUBLE,i%(p-2)+2,RESULTS,MPI_COMM_WORLD,&status);

		}
		gettimeofday(&tv2,NULL);
		printf("%d: %ld\n",p,(tv2.tv_sec - tv1.tv_sec)*1000000 + tv2.tv_usec - tv1.tv_usec);
	}
	else{
		// I am a worker
		MPI_Recv(&dimx,1,MPI_INT,EMETTITORE,PARAMETERS,MPI_COMM_WORLD,&status);
		MPI_Recv(&dimy,1,MPI_INT,EMETTITORE,PARAMETERS,MPI_COMM_WORLD,&status);
		MPI_Recv(&test_g.A,1,MPI_DOUBLE,EMETTITORE,PARAMETERS,MPI_COMM_WORLD,&status);
		MPI_Recv(&test_g.x_0,1,MPI_DOUBLE,EMETTITORE,PARAMETERS,MPI_COMM_WORLD,&status);
		MPI_Recv(&test_g.y_0,1,MPI_DOUBLE,EMETTITORE,PARAMETERS,MPI_COMM_WORLD,&status);
		MPI_Recv(&test_g.sigma_x,1,MPI_DOUBLE,EMETTITORE,PARAMETERS,MPI_COMM_WORLD,&status);
		MPI_Recv(&test_g.sigma_y,1,MPI_DOUBLE,EMETTITORE,PARAMETERS,MPI_COMM_WORLD,&status);
		MPI_Recv(&test_g.c,1,MPI_DOUBLE,EMETTITORE,PARAMETERS,MPI_COMM_WORLD,&status);
		
		/* inizializzo gli altri valori della struct */
		test_g.a = 0;
		test_g.b = 0;
		
		dim = dimx*dimy;
		cropped = (unsigned char*) malloc(dim);
		
		num_image = STREAMLENGTH / (p-PS);		
		if(STREAMLENGTH % (p-PS) > my_rank-PS){
			num_image++;
		}

#if DEBUG		
		printf("Sono il processo %d  e devo ricevere %d immagini\n",my_rank,num_image);
#endif		
		/* CICLO SULLE IMMAGINI */
		for (i=0;i< num_image ;i++){
			MPI_Recv(cropped,dim,MPI_UNSIGNED_CHAR,EMETTITORE,IMAGE,MPI_COMM_WORLD,&status);
			iteration(cropped,dimx,dimy,&test_g);
#if DEBUG
			printf("PROCESSO %d IMMAGINE %d %f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", my_rank,i,(&test_g)->A, (&test_g)->x_0,
			 (&test_g)->y_0, (&test_g)->sigma_x, (&test_g)->sigma_y, (&test_g)->a, (&test_g)->b, (&test_g)->c);
#endif

			MPI_Send(fit, DIM_FIT, MPI_DOUBLE, COLLETTORE , RESULTS, MPI_COMM_WORLD);
		}

	}
	
	MPI_Finalize();
	return 0;
}
