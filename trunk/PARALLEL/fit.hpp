#ifndef __FIT__H
#define __FIT__H

#define GAUSSIAN 'G'
#define AIRY 'A'

#define MASSIMO 255

#include <stdio.h>
#include "math.h"

#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>

#include <time.h>

typedef struct fit {

  	/** function type*/
  	char type;

  	/** fit parameters */
	double A;
  	double x_0;
  	double y_0;
  	double sigma_x;
  	double sigma_y;
  	double a;
  	double b;
  	double c;
} fit_t;

int iteration(const unsigned char* data,int x,int y,fit_t* results);

void iteration2(const unsigned char* data,int x,int y,fit_t* results);

unsigned char* createMask(unsigned char* image,int w,int h,int max,int min,double filter);

void centroid(unsigned char* image,int w,int h,double* x,double* y,double* sigma_x,double* sigma_y);

unsigned char* cropImage(const unsigned char *input, int w,int h,int x1,int x2,int y1,int y2);

double evaluateGaussian(fit_t* gaussian,int x, int y);

void maxmin(unsigned char* image,int w,int h,int* max,int* min);

#endif
