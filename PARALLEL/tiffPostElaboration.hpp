#ifndef __TIFFPOSTELABORATION__H
#define __TIFFPOSTELABORATION__H

#include <tiffio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void writeImage(unsigned char* image,char *dest, int w, int h);

unsigned char* readTIFF(int* width,int* height,int* max,int *min,char* link);

void octaveVectorPort(unsigned char *data,int length);

void octaveMatrixPort(unsigned char *data,int width,int length);

void matlabMatrixPort(unsigned char *data,int width,int length);

#endif
