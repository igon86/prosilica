#ifndef __IMAGE__H
#define __IMAGE__H

#include "fit.h"

/* mean and maximum deviation acceptable for the gaussian amplitude */
#define AVERAGE 200
#define DEVIATION 50

/* Create a gaussian Image */
unsigned char* createGaussian(int width, int height);

/* Create the image representing the Gaussian */
unsigned char* createMatrix (int length, int width, double* input);

/* Write the image in a .TIFF file (useful for debugging) */
void writeImage (unsigned char* image, char *dest, int w, int h);

/* Create a gaussian Image */
unsigned char* createImage(const char* parameters, int* width, int* height);

#endif
