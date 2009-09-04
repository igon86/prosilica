#include <tiffio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void writeImage(unsigned char* a,char *dest, int w, int h);

unsigned char* readTIFF(int* width,int* height,int* max,char* link);

unsigned char* createMask(unsigned char* image,int w,int h,int max,double filter);

void centroid(unsigned char* image,int w,int h,double* x,double* y);

void octaveVectorPort(unsigned char *data,int length);

void octaveMatrixPort(unsigned char *data,int width,int length);

void matlabMatrixPort(unsigned char *data,int width,int length);
