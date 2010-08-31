#include "fit.h"
#include "parallel.h"

static unsigned char *crop = NULL;

extern int my_rank;

/***************************************************************************************************************
							Centroid
 ****************************************************************************************************************/

/** image need to be the output of a createMask function
 both the center position and the dimension of the centroid depends
 on the filter parameter of the createMask function previously used */

void centroid(unsigned char *image, int w, int h, double *x, double *y, double *sigma_x, double *sigma_y) {	
	int npixels = w * h;
	// INCAPIBILE
	int* counth = (int*) malloc (sizeof(int) * h);
	int* countw = (int*) malloc (sizeof(int) * w);	
	int count = 0, i = 0;
	double w_center = 0.0, h_center = 0.0;
	int left_border = 0, right_border = 0;
	int down_border = 0, up_border = 0;

    	for (i = 0; i < h; i++)
		counth [i] = 0;
    	for (i = 0; i < w; i++)
		countw [i] = 0;
	for (i = 0; i < npixels; i++) {
		if (image [i]) {
			++count;
			++countw [i % w];
			++counth [i / w];
		}
    	}
	
	/* W CENTER */
	for (i = 0; i < w; i++) {
		w_center = w_center + ((double) countw [i] / count) * (i + 1);
		if (!left_border && countw [i])
			left_border = i;
		if (left_border && !right_border && !countw [i])
			right_border = i;
	}
	
	/* H CENTER */
	for (i = 0; i < h; i++) {
		h_center = h_center + ((double) counth [i] / count) * (i + 1);
		if (!down_border && counth[i])
			down_border = i;
		if (down_border && !up_border && !counth [i])
			up_border = i;
	}
	*x = w_center;
	*y = h_center;
	*sigma_x = (right_border - left_border) / 2.0;
	*sigma_y = (up_border - down_border) / 2.0;

	free(counth);
	free(countw);
}


/***************************************************************************************************************
								Create Matrix
 ****************************************************************************************************************/
 
/** 
Return the image of the gaussian, dimension of the image are given by ( length, width ) parameters while 
parameters of the gaussian are stored in the array input.

\param	(length, width)		dimension of the returned image
\param	input				array containing gaussian parameters

\retval		representation of the gaussian as a 8bit image (unsigned char)
*/ 
unsigned char * createMatrix (int length, int width, double* input){
	int i = 0, j = 0;	
	int dim = length * width;
	unsigned char* matrix = (unsigned char*) malloc (dim);
	unsigned char* p = matrix;	
	/* build the image */
	for (i = 0;i < length; i++)
		for(j = 0; j < width; j++)
			*p++ = (unsigned char) evaluateGaussian(input, j, i);
	return matrix;
}

/***************************************************************************************************************
										Cookie Mask
 ****************************************************************************************************************/

/** 
Return the luminosity mask of an image. Pixels above a certain threshold of luminosity (specified 
by the filter parameter) are white while all the others are black.

\param		image		pointer of the image
\param		w,h			width and heigth of the image
\param		max,min		maximimum and minumum luminosity of the image
\param		filter		determines the threshold for the mask, 0 < filter < 1

\retval		representation of the image mask as a 8bit image (unsigned char)
*/ 
unsigned char *createMask(unsigned char *image, int w, int h, int max, int min, double filter) {
	int threshold = (int) (filter * (max - min));
	int i = 0;
    	int npixels = w * h;
	unsigned char *cookie = (unsigned char*) malloc(npixels);
	for (i = 0; i < npixels; i++) {
		unsigned char temp = image[i];
		if (temp > threshold)
			cookie[i] = MAXIMUM;
		else
			cookie[i] = 0;
	}
	return cookie;
}

/***************************************************************************************************************
										Crop Function
 ****************************************************************************************************************/

/** 
Return a portion of the input image as specified by the input coordinates and x,y dimensions

\param		image		pointer of the input image
\param		w,h			width and heigth of the input image
\param		x0,y0		coordinates of the starting point of the crop
\param		dimx,dimy	dimensione of the crop 

\retval		representation of the crop as a 8bit image (unsigned char)
*/ 

unsigned char *cropImage(const unsigned char *input, int w, int h, int x0, int y0, int dimx, int dimy) {
	int count = 0, i = 0;
	int dimension = dimx * dimy;
	if(crop != NULL)
		free(crop);
	crop = (unsigned char*) malloc(dimension);
	for (i = 0; i < w*h; i++)
		if (i % w >= x0 && i % w <= x0+dimx-1 && i / w >= y0 && i / w <= y0 + dimy-1)
			crop[count++] = input[i];
	return crop;
}

/***************************************************************************************************************
								Evaluate Gaussian at Coordinates (x,y)
 ****************************************************************************************************************/

/** 
Return the value of a given gaussian in a given point 

\param		gaussian	parameters of the gaussian
\param		x,y			coordinates of the point

\retval		value of the gaussian in the point of given coordinates
*/ 
double evaluateGaussian(double* gaussian, int x, int y) {
	double slope = gaussian[PAR_a] * x + gaussian[PAR_b] * y + gaussian[PAR_c];
	double x_arg = pow(((double) x - gaussian[PAR_X]), 2.0) / pow(gaussian[PAR_SX], 2.0);
	double y_arg = pow(((double) y - gaussian[PAR_Y]), 2.0) / pow(gaussian[PAR_SY], 2.0);
	double arg = -(x_arg + y_arg);
	return gaussian[PAR_A] * exp(arg) + slope;
}

/***************************************************************************************************************
									Fit Algorithm
 ****************************************************************************************************************/

/** 
Given an image and an array of previous results return the Gauss matrix and vector

\param		data		image of the gaussian
\param		w,h			width and height of the image
\param		results		parameters of the gaussian at the previous step
\param		matrice		gauss matrix
\param		vettore		gauss vector

*/ 
void procedure (const unsigned char *data, int w, int h, double * results, gsl_matrix_view matrice,gsl_vector_view vettore) {

	int npixels = w * h;
	
	int i = 0, x = 0, y = 0, base = 0;
	double diff_x = 0.0, diff_y = 0.0, frac_x = 0.0, frac_y = 0.0, sig2x = 0.0, sig2y = 0.0, dexp = 0.0;
	
	/* vectors used in the main loop with relative gsl_view */
	gsl_matrix_view gsl_M;
	gsl_vector_view differenze;
	
	double *M = (double*) malloc(sizeof(double) * DIM_FIT * npixels);
	double *diff = (double*) malloc (sizeof(double) * npixels);
	
	gsl_M = gsl_matrix_view_array(M, npixels, DIM_FIT);
	differenze = gsl_vector_view_array(diff, npixels);
	
	/* Task over the image, for every pixels its coordinates are determined first
	then the actual data is compared with prediction and used to construct the matrix M and
	the vector diff */
	for (i = 0; i < npixels; i++) {
		x = (i + 1) % w;

		/** in case of a data parallel computation the worker owns only a portion of rows of the image
		thus the correct heigth of the examined point is determined using the rank of the worker */
			
#ifdef FARM
		y = ((i + 1) / w);
#endif

#ifdef DATA_PARALLEL
		y = ((i + 1) / w ) + h * my_rank;
#endif
			
		base = i * DIM_FIT;
		diff[i] = data[i] - evaluateGaussian(results, x, y);
			
		diff_x = x - results[PAR_X];
		diff_y = y - results[PAR_Y];
		sig2x = pow(results[PAR_SX], 2);
		sig2y = pow(results[PAR_SY], 2);
		frac_x = pow(diff_x, 2) / sig2x;
		frac_y = pow(diff_y, 2) / sig2y;
		dexp = exp(frac_x + frac_y);
			
		M[base] = 1 / dexp;
		M[base + 1] = (results[PAR_A] * (2 * x - 2 * results[PAR_X])) / (sig2x * dexp);
		M[base + 2] = (results[PAR_A] * (2 * y - 2 * results[PAR_Y])) / (sig2y * dexp);
		M[base + 3] = (2 * results[PAR_A] * pow(diff_x, 2)) / (pow(results[PAR_SX], 3) * dexp);
		M[base + 4] = (2 * results[PAR_A] * pow(diff_y, 2)) / (pow(results[PAR_SY], 3) * dexp);
		M[base + 5] = x;
		M[base + 6] = y;
		M[base + 7] = 1.0;
	}
		
	/* Compute matrix = M'*M */
	gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1.0, &gsl_M.matrix, &gsl_M.matrix, 0.0, &matrice.matrix);
	/* Compute vector = M'*diff */
	gsl_blas_dgemv(CblasTrans, 1.0, &gsl_M.matrix, &differenze.vector, 0.0, &vettore.vector);
	
}

/***************************************************************************************************************
							Calculate Max & Min Luminosity of an Image
 ****************************************************************************************************************/

/** 
Given an image it determines the maximum and minimum luminosity of the image

\param		image		image to be analyzed
\param		w,h			width and height of the image
\param		max			maximum of luminosity
\param		min			minimum luminosity

*/
void maxmin(unsigned char *image, int w, int h, int *max, int *min) {
    int npixels = w * h, i = 0;
    *max = 0;
    *min = MAXIMUM;
    for (i = 0; i < npixels; i++) {
		if (image[i] > *max)
			*max = image[i];
		if (image[i] < *min)
			*min = image[i];
    }
}

/***************************************************************************************************************
						Write mono8 black and white TIFF
****************************************************************************************************************/

/** 
Given an image represented by an unsigned char matrix it writes the relative TIFF image
on a specified file

\param		image		image to be written on file
\param		dest		pathname of the output TIFF file
\param		w,h			width and height of the image

*/
void writeImage(unsigned char* image, char* dest, int w, int h) {
		
		TIFF* out = TIFFOpen(dest, "w");
		tsize_t linebytes;
		uint32 row = 0;
		/* buffer used to store the row of pixel information for writing to file */
		unsigned char *buf = NULL;
		/* 8bit image */
		int sampleperpixel = 1;
		
		TIFFSetField (out, TIFFTAG_IMAGEWIDTH, w);						/* set the width of the image */
		TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);						/* set the height of the image */
		TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);		/* set number of channels per pixel */
		TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);					/* set the size of the channels */
		TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    /* set the origin of the image */
		
		/* Some other essential fields to set */
		TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
		
		/* determining the dimension of a single row */
		linebytes = sampleperpixel * w;     /* length in memory of one row of pixel in the image */
		
		/* Allocating memory to store the pixels of current row */
		if (TIFFScanlineSize(out) == linebytes)
			buf =(unsigned char *)_TIFFmalloc(linebytes);
		else
			buf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(out));
				
		/* Set the strip size of the file to be size of one row of pixels */
		TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, w*sampleperpixel));
		
		/* Writing image to the file one strip at a time */
		for (row = 0; row < h; row++) {
			memcpy(buf, &image[(h-row-1)*linebytes], linebytes);		/* tricky index */
			if (TIFFWriteScanline(out, buf, row, 0) < 0) break;
		}
		(void) TIFFClose(out);
		if (buf) _TIFFfree(buf);
}

/***************************************************************************************************************
						Initialization of the Fit
****************************************************************************************************************/

int initialization(char* parameter, double* input, double* fit, unsigned char** matrix, unsigned char** cropped, int* dimx, int* dimy, int p){
		
	/* parameters for the cookie cutter */
	double x0 = 0.0, y0 = 0.0;
	double FWHM_x = 0.0, FWHM_y = 0.0;
	int span_x = 0, span_y = 0;
	int x = 0 , y = 0;
	/* File conteining parameters*/
	FILE* parameters = NULL;
	/* width and length of the input image */
	int width = 0, length = 0;
	/* parameters fro create the mask */
	int max = 0, min = 0;
	/* pixel mask for reduce the dimension of the region to analyze */
	unsigned char *mask = NULL;

	int i = 0;
		/* reading the input parameters */		
		if((parameters = fopen(parameter, "r")) == NULL){
			fprintf(stderr, "File not valid");
			exit(EXIT_FAILURE);		
		}
	
		/* initialize the dimension of the image */
		if(fscanf(parameters, "%d\t%d\t", &width, &length) == 0){
			fprintf(stderr, "File not valid");
			exit(EXIT_FAILURE);							
		}
		/* initialize the fit of the Gaussian */
		for(i = 0; i < DIM_FIT; i++){
			if(fscanf(parameters, "%lf\t", &input[i]) == 0){
				fprintf(stderr, "File not valid");
				exit(EXIT_FAILURE);							
			}		
			fit[i] = 0;
		}

		/* image representing the Gaussian fit */
		*matrix = createMatrix( length, width, input);
	
		/* writing the image to be fitted in a FIT file */
#if DEBUG
		writeImage((unsigned char *) *matrix,(char *) "gaussiana.tiff", width, length);
#endif	
		maxmin( (unsigned char*) *matrix, width, length, &max, &min);

#if DEBUG	
		printf("MAX: %d MIN: %d\n", max, min);
#endif
	
		/* a pixel mask is created in order to reduce the dimensione of the region to analyze with the centroid */
		mask = createMask( (unsigned char*) *matrix, width, length, max, min, CROP_PARAMETER);
	
#if DEBUG
		writeImage(mask, (char *) "mask.tiff", width, length);
#endif
	
		centroid(mask, width, length, &x0, &y0, &FWHM_x, &FWHM_y);
	
#if DEBUG
		printf("centro in %f - %f\nCon ampiezza %f e %f\n", x0, y0, FWHM_x, FWHM_y);
#endif
	
		free(mask);
	
		/* inizialization for the diameter of the gaussian*/
		span_x = (int) (2 * FWHM_x);
		span_y = (int) (2 * FWHM_y);

		*dimx = 2 * span_x + 1;
		*dimy = 2 * span_y + 1;
		
#ifdef DATA_PARALLEL
		while(*dimx % p != 0)
			++*dimx;
		while(*dimy % p != 0)
			++*dimy;
#endif

		/* inizialization of the position coordinates */
		x = (int) x0;
		y = (int) y0;
	
		/**
		 inizialization of the fit of the Gaussian
		 NOTE: the coordinates of the position (x,y) are relative to the cropped portion of the image.
		 The value of span_x, which is approximately the diameter of the gaussian, is generally not as bad
		 as you may think to start the fit.
		 */
		fit[PAR_A] = max;
		fit[PAR_X] = span_x;
		fit[PAR_Y] = span_y;
		fit[PAR_SX] = FWHM_x;
		fit[PAR_SY] = FWHM_y;
		fit[PAR_c] = min;	

#if DEBUG
	printf("%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", fit[PAR_A], fit[PAR_X] + x - span_x, fit[PAR_Y] + y - span_y, fit[PAR_SX], fit[PAR_SY], fit[PAR_a], fit[PAR_b], fit[PAR_c]);
#endif
		*cropped = cropImage((unsigned char*) *matrix, width, length, x - span_x, y - span_y, *dimx, *dimy);	
	
#if DEBUG
		writeImage(*cropped, (char *) "./CROP.tiff", *dimx, *dimy);
#endif

	return 0;
}
