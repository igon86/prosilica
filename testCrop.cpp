#include "tiffPostElaboration.hpp"
#include <stdio.h>
#include <stdlib.h>

int main(int argc,char *argv[]){
	int w;
	int h;
	int max;
	unsigned char* immagine = readTIFF(&w,&h,&max,argv[1]);
	printf("LETTA");
	fflush(stdout);
	writeImage(immagine,"copy.tiff",w,h);
	unsigned char* mask = createMask(immagine,w,h,max,0.55);
	writeImage(mask,"mannaggia.tiff",w,h);
	double x;
	double y;
	centroid(mask,w,h,&x,&y);
	printf("centro in %f - %f",x,y);
	return 0;
}