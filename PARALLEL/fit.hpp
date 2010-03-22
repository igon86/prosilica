#ifndef __FIT__H
#define __FIT__H

#define GAUSSIAN 'G'
#define AIRY 'A'

#define DIM_FIT 8
#define MASSIMO 255

#include <stdio.h>
#include "math.h"

#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>

#include <time.h>

#define PAR_A 0
#define PAR_X 1
#define PAR_Y 2
#define PAR_SX 3
#define PAR_SY 4
#define PAR_a 5
#define PAR_b 6
#define PAR_c 7
/*typedef struct fit {

  	char type;
	double A;
  	double x_0;
  	double y_0;
  	double sigma_x;
  	double sigma_y;
  	double a;
  	double b;
  	double c;
} fit_t;*/

int iteration(const unsigned char* data,int x,int y, double* results);

unsigned char* createMask(unsigned char* image,int w,int h,int max,int min,double filter);

void centroid(unsigned char* image,int w,int h,double* x,double* y,double* sigma_x,double* sigma_y);

unsigned char* cropImage(const unsigned char *input, int w,int h,int x1,int x2,int y1,int y2);

double evaluateGaussian(double* gaussian,int x, int y);

void maxmin(unsigned char* image,int w,int h,int* max,int* min);

#endif
