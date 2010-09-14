#ifndef __FIT__H
#define __FIT__H

#include <stdio.h>
#include <math.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <stdlib.h>
#include <tiffio.h>
#include <string.h>

/* dimension of the fit of a Gaussian */
#define DIM_FIT 8

#define CROP_PARAMETER 0.5
#define MAXIMUM 255
#define TRUE 1==1

/* Define the parameters of a Gaussian fit*/
enum{
	PAR_A = 0,  /* amplitude Gaussian            */
	PAR_X,  	/* x position Gaussian           */
	PAR_Y,  	/* y position Gaussian           */
	PAR_SX, 	/* variance x axis Gaussian      */
	PAR_SY, 	/* variance y axis Gaussian      */
	PAR_a,  	/* first parameter of the plane  */
	PAR_b,  	/* second parameter of the plane */
	PAR_c  		/* third parameter of the plane  */
};

/* Find maximum and minumum luminosity of a 8bit image */
void maxmin(unsigned char *image, int w, int h, int *max, int *min);

/* Create the pixel mask of the image in order to reduce the dimension */
unsigned char* createMask (unsigned char* image, int w, int h, int max, int min, double filter);

/* Evaluate the centroid */
void centroid (unsigned char* image, int w, int h, int* x, int* y, int* sigma_x, int* sigma_y);

/* Task over the image */
void procedure (const unsigned char* data, int x, int y,  double * results, gsl_matrix_view matrice,gsl_vector_view vettore,int offset);

/* Result update */
void postProcedure(gsl_matrix_view matrice,gsl_vector_view vettore, double* fit);

/* Evaluate the Gaussian at coordinates (x, y) */
double evaluateGaussian (double* gaussian, int x, int y);

/* Init fit buffers */
void initBuffers(int npixels);

/* Estimate the parameters of the gaussian */
void initialization(unsigned char *matrix,int width,int height, double *fit);

#endif
