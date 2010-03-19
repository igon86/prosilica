#include <stdio.h>

#include "tiffPostElaboration.hpp"
#include "fit.hpp"
#include <mpi.h>

#define OUTPUT_MATRIX "gaussiana.tiff" 
#define CROP_PARAMETER 0.5
#define STREAMLENGTH 10

#define EMETTITORE 0
#define DIM 0
#define IMAGE 1

extern FILE *risultati;

int main(int argc, char* argv[]){
	
	FILE* parameters;
	
	/* MPI VARIABLES */
	int my_rank,p;
	MPI_Status status;
	
	/* parameters of the gaussian */
	int width;
	int length;
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
	
	/* parameters for the cookie cutter */
	double x0,y0;
	double FWHM_x,FWHM_y;
	int span_x,span_y;
	int dimx,dimy;
	int x,y;
	
	/* gaussian struct */
	fit_t results,test_g;
	
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
		fscanf(parameters,"%d\t%d\t",&width,&length);
		fscanf(parameters,"%lf\t%lf\t%lf\t",&amplitude,&x_0,&y_0);
		fscanf(parameters,"%lf\t%lf\t",&sigma_x0,&sigma_y0);
		fscanf(parameters,"%lf\t%lf\t%lf",&a_0,&b_0,&c_0);
	
		/* GAUSSIAN_MATRIX */
		unsigned char matrix[length][width];
	
#if DEBUG
		/* STAMPA DI DEBUG DEI PARAMETRI DELLA GAUSSIANA DA INTERPOLARE */
		printf("Dimensioni della matrice: %d %d\n",width,length);
		printf("Ampiezza: %f\nPosizione asse X: %f\nPosizione asse Y: %f\n",amplitude,x_0,y_0);
		printf("Varianza asse X: %f\nVarianza asse Y: %f\n",sigma_x0,sigma_y0);
		printf("A(x): %f\nB(y): %f\nC: %f\n",a_0,b_0,c_0);
#endif	
	
		/* ASSEGNAZIONE ALLA STRUCT */
		results.A = amplitude;
		results.x_0 = x_0;
		results.y_0 = y_0;
		results.sigma_x = sigma_x0;
		results.sigma_y = sigma_y0;
		results.a = a_0;
		results.b = b_0;
		results.c = c_0;	
	
		/* COSTRUZIONE DELL'IMMAGINE */
		for (i=0;i<length;i++){
			for(j=0;j<width;j++){
				temp = (int) evaluateGaussian(&results,j,i);
				//printf("%d\n",temp);
				matrix[i][j] = temp;
			}
		}
	
		/* WRITING THE IMAGE TO BE FITTED ON A TIFF FILE */
		writeImage((unsigned char *)matrix,(char *) OUTPUT_MATRIX, width, length);
	
		maxmin( (unsigned char*) matrix, width, length, &max, &min);
	
		printf("MAX: %d MIN: %d\n", max, min);
	
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
		test_g.A = max;
		test_g.x_0 = span_x;
		test_g.y_0 = span_y;
		test_g.sigma_x = FWHM_x;
		test_g.sigma_y = FWHM_y;
		test_g.a = 0;
		test_g.b = 0;
		test_g.c = min;
	
		fprintf(risultati, "%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", (&test_g)->A, (&test_g)->x_0 + x - span_x, (&test_g)->y_0 + y - span_y, (&test_g)->sigma_x, (&test_g)->sigma_y, (&test_g)->a, (&test_g)->b, (&test_g)->c);
	
	
		/* THIS PART CAN BE ITERATIVE */
	
		unsigned char *cropped = cropImage((unsigned char*) matrix, width, length, x - span_x, x + span_x, y - span_y, y + span_y);
	
#if DEBUG
		writeImage(cropped, (char *) "./CROP.tiff", dimx, dimy);
#endif
		// calcolo la dimensione e la comunico ai worker
		dim=dimx*dimy;
		for(i=1;i<p;i++){
			MPI_Send(&dimx, i, MPI_INT, i, DIM, MPI_COMM_WORLD);
			MPI_Send(&dimy, i, MPI_INT, i, DIM, MPI_COMM_WORLD);
		}
		/*
		for(i=0; i < STREAMLENGTH; i++){
			//MPI_Send(cropped, dim, MPI_BYTE, i%(p-1)+1, IMAGE, MPI_COMM_WORLD);		
			//iteration(.l,jed, dimx, dimy, &test_g);
		
			fprintf(risultati, "%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", (&test_g)->A, (&test_g)->x_0 + x - span_x, (&test_g)->y_0 + y - span_y, (&test_g)->sigma_x, (&test_g)->sigma_y, (&test_g)->a, (&test_g)->b, (&test_g)->c);
		}
		*/
	}else{
		// I am a worker
		MPI_Recv(&dimx,1,MPI_INT,EMETTITORE,DIM,MPI_COMM_WORLD,&status);
		MPI_Recv(&dimy,1,MPI_INT,EMETTITORE,DIM,MPI_COMM_WORLD,&status);
	}
	
	MPI_Finalize();
	return 0;
}
