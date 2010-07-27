#ifndef __FIT_DP__H
#define __FIT_DP__H

#include <stdio.h>
#include <math.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <tiffio.h>
#include <stdlib.h>
#include <string.h>

/* dimension of the fit of a Gaussian */
#define DIM_FIT 8

/* Define the parameters of a Gaussian fit*/
enum{
	PAR_A = 0,  	/* amplitude Gaussian            */
	PAR_X,  	/* x position Gaussian           */
	PAR_Y,  	/* y position Gaussian           */
	PAR_SX, 	/* variance x axis Gaussian      */
	PAR_SY, 	/* variance y axis Gaussian      */
	PAR_a,  	/* first parameter of the plane  */
	PAR_b,  	/* second parameter of the plane */
	PAR_c  		/* third parameter of the plane  */
};

#define CROP_PARAMETER 0.5
#define MAXIMUM 255
#define TRUE 1==1

/* Create the image representing the Gaussian fit */
unsigned char* createMatrix (int length, int width, double* result);

void maxmin (unsigned char* image, int w, int h, int* max, int* min);

/* Create the pixel mask of the image in order to reduce the dimension */
unsigned char* createMask (unsigned char* image, int w, int h, int max, int min, double filter);

/* Evaluate the centroid */
void centroid (unsigned char* image, int w, int h, double* x, double* y, double* sigma_x, double* sigma_y);

/* Reduce the dimension of the image */
unsigned char* cropImage (const unsigned char *input, int w, int h, int x1, int x2, int y1, int y2);

/* Task over the image */
int procedure (const unsigned char* data, int x, int y, double * results, gsl_matrix_view matrice,gsl_vector_view vettore);

/* Evaluate the Gaussian at coordinates (x, y) */
double evaluateGaussian (double* gaussian, int x, int y);

/* Write the image in a .TIFF file named dest (for debugging) */
void writeImage (unsigned char* image, char *dest, int w, int h);

/* Initialize of the fit */
int initialization(char* parameters, double* results, double* fit, unsigned char** matrix, unsigned char** cropped, int* dimx, int* dimy);
#endif
