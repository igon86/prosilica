#include "tiffPostElaboration.hpp"
#include "fit.hpp"
#include <stdio.h>
#include <stdlib.h>

int main(int argc,char *argv[]){
	int w;
	int h;
	int max;
	int min;

	unsigned char* immagine = readTIFF(&w,&h,&max,&min,argv[1]);
	printf("MAX: %d MIN: %d\n",max,min);
	writeImage(immagine,"copy.tiff",w,h);
	unsigned char* mask = createMask(immagine,w,h,max,min,0.5);
	writeImage(mask,"mask.tiff",w,h);
	double x0;
	double y0;
	double FWHM_x;
	double FWHM_y;
	centroid(mask,w,h,&x0,&y0,&FWHM_x,&FWHM_y);
	printf("centro in %f - %f\nCon ampiezza %f e %f\n",x0,y0,FWHM_x,FWHM_y);
	
	int span_x = (int) (2*FWHM_x);
	int span_y = (int) (2*FWHM_y);
	int x = (int) x0;
	int y = (int) y0;
	unsigned char* cropped = cropImage(immagine,w,h,x-span_x,x+span_x,y-span_y,y+span_y);
	int dimx = 2*span_x+1;
	int dimy = 2*span_y+1;
	writeImage(cropped,"crop.tiff",dimx,dimy);
	
	fit_t test_g;
	test_g.type = GAUSSIAN;
	test_g.A = max;
	test_g.x_0 = span_x;
	test_g.y_0 = span_y;
	test_g.sigma_x = FWHM_x;
	test_g.sigma_y = FWHM_y;
	test_g.a = 0;
	test_g.b = 0;
	test_g.c = min;
	
	int dimension = dimx*dimy;
	
	unsigned char* pred = new unsigned char [dimension];
	int temp;
	for (int i=0; i < dimension; i++){
		temp = (int) evaluateGaussian(&test_g,i%dimx,i/dimx);
		pred[i] = temp;
	}
	printf("\n");
	writeImage(pred,"pred.tiff",dimx,dimy);
	iteration(cropped,dimx,dimy,&test_g);
	return 0;
}
