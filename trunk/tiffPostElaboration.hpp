#include <tiffio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


bool writeImage(unsigned char* a,char *dest, int w, int h);

void octaveVectorPort(unsigned char *data,int length);

void octaveMatrixPort(unsigned char *data,int width,int length);

void matlabMatrixPort(unsigned char *data,int width,int length);
