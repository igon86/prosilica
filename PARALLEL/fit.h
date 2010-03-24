#ifndef __FIT__H
#define __FIT__H

#include <stdio.h>
#include <math.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <tiffio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* dimension of the fit */
#define DIM_FIT 8

#define MASSIMO 255

#define PAR_A 0  	/* amplitude Gaussian            */
#define PAR_X 1  	/* x position Gaussian           */
#define PAR_Y 2  	/* y position Gaussian           */
#define PAR_SX 3 	/* variance x axis Gaussian      */
#define PAR_SY 4 	/* variance y axis Gaussian      */
#define PAR_a 5  	/* first parameter of the plane  */
#define PAR_b 6  	/* second parameter of the plane */
#define PAR_c 7  	/* third parameter of the plane  */

int iteration (const unsigned char* data,int x,int y, double* results);

unsigned char* createMask (unsigned char* image, int w, int h, int max, int min, double filter);

unsigned char* createMatrix (int length, int width, double* result);

void centroid (unsigned char* image, int w, int h, double* x, double* y, double* sigma_x, double* sigma_y);

unsigned char* cropImage (const unsigned char *input, int w, int h, int x1, int x2, int y1, int y2);

double evaluateGaussian (double* gaussian, int x, int y);

void maxmin (unsigned char* image, int w, int h, int* max, int* min);

void writeImage (unsigned char* image, char *dest, int w, int h);

#endif
