#include "fit.h"
#include "parallel.h"
#include "image.h"

/* vectors used in the main loop with relative gsl_view */
static gsl_matrix_view gsl_M;
static gsl_vector_view gsl_diff;
static double* M = NULL;
static double* diff = NULL;

extern int my_rank;
extern int p;

/***************************************************************************************************************
 Post Procedure
 ****************************************************************************************************************/

/**
 Given a pixel mask of a gaussian image, represented by an unsigned char matrix, it estimates the center and the variance
 of the gaussian along the x and y axis
 
 \param		image				image of the gaussian
 \param		w,h					width and height of the image
 \param		x,y					estimated cordinates
 \param		sigma_x,sigma_y		estimated variance in the x and y axis
 
 */
void postProcedure(gsl_matrix_view matrice,gsl_vector_view vettore, double* fit){
	/* data for the LU solver */
    gsl_vector *delta = gsl_vector_alloc(DIM_FIT);
    gsl_permutation *permutation = gsl_permutation_alloc(DIM_FIT);
	int j,error;
	
	gsl_linalg_LU_decomp(&matrice.matrix, permutation, &error);	
	gsl_linalg_LU_solve(&matrice.matrix, permutation, &vettore.vector, delta);
	
	for (j = 0; j < DIM_FIT; j++)
		fit[j] = fit[j] + gsl_vector_get(delta, j);
}

/***************************************************************************************************************
 Centroid
 ****************************************************************************************************************/

/**
 Given a pixel mask of a gaussian image, represented by an unsigned char matrix, it estimates the center and the variance
 of the gaussian along the x and y axis
 
 \param		image				image of the gaussian
 \param		w,h					width and height of the image
 \param		x,y					estimated cordinates
 \param		sigma_x,sigma_y		estimated variance in the x and y axis
 
 */
void centroid(unsigned char *image, int w, int h, int *x, int *y,
			  int *sigma_x, int *sigma_y)
{
    int npixels = w * h;
    int *counth = (int *) malloc(sizeof(int) * h);
    int *countw = (int *) malloc(sizeof(int) * w);
    int count = 0, i = 0;
    double w_center = 0.0, h_center = 0.0;
    int left_border = 0, right_border = 0;
    int down_border = 0, up_border = 0;
	
    for (i = 0; i < h; i++)
		counth[i] = 0;
    for (i = 0; i < w; i++)
		countw[i] = 0;
    for (i = 0; i < npixels; i++) {
		if (image[i]) {
			++count;
			++countw[i % w];
			++counth[i / w];
		}
    }
	
    /* W CENTER */
    for (i = 0; i < w; i++) {
		w_center = w_center + ((double) countw[i] / count) * (i + 1);
		if (!left_border && countw[i])
			left_border = i;
		if (left_border && !right_border && !countw[i])
			right_border = i;
    }
	
    /* H CENTER */
    for (i = 0; i < h; i++) {
		h_center = h_center + ((double) counth[i] / count) * (i + 1);
		if (!down_border && counth[i])
			down_border = i;
		if (down_border && !up_border && !counth[i])
			up_border = i;
    }
    *x = (int) w_center;
    *y = (int) h_center;
    *sigma_x = right_border - left_border;
    *sigma_y = up_border - down_border;
	
    free(counth);
    free(countw);
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
unsigned char *createMask(unsigned char *image, int w, int h, int max,
						  int min, double filter)
{
    int threshold = (int) (filter * (max - min));
    int i = 0;
    int npixels = w * h;
    unsigned char *cookie = (unsigned char *) malloc(npixels);
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
 Evaluate Gaussian at Coordinates (x,y)
 ****************************************************************************************************************/

/**
 Return the value of a given gaussian in a given point
 
 \param		gaussian	parameters of the gaussian
 \param		x,y			coordinates of the point
 
 \retval		value of the gaussian in the point of given coordinates
 */
double evaluateGaussian(double *gaussian, int x, int y)
{
    double slope =
	gaussian[PAR_a] * x + gaussian[PAR_b] * y + gaussian[PAR_c];
    double x_arg =
	pow(((double) x - gaussian[PAR_X]), 2.0) / pow(gaussian[PAR_SX],
												   2.0);
    double y_arg =
	pow(((double) y - gaussian[PAR_Y]), 2.0) / pow(gaussian[PAR_SY],
												   2.0);
    double arg = -(x_arg + y_arg);
    return gaussian[PAR_A] * exp(arg) + slope;
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
void maxmin(unsigned char *image, int w, int h, int *max, int *min)
{
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
 Init Fit Buffers
 ****************************************************************************************************************/
 
 /**
 Computes the gradients of the gaussian relative to the given coordinates (x,y), results
 are stored in the given gradient matrix M
 
 \param		data		image of the gaussian
 \param		w,h			width and height of the image
 \param		results		parameters of the gaussian at the previous step
 \param		matrice		gauss matrix
 \param		vettore		gauss vector
 
 */
 void initBuffers(int npixels){
#ifdef DEBUG
	printf("Sono il processo %d e mi sono stati chiesti %d pixels\n",p,npixels);
#endif
	M = (double *) malloc(sizeof(double) * DIM_FIT * npixels);
    diff = (double *) malloc(sizeof(double) * npixels);
	gsl_M = gsl_matrix_view_array(M, npixels, DIM_FIT);
    gsl_diff = gsl_vector_view_array(diff, npixels);
 }

/***************************************************************************************************************
 Gaussian Gradients
 ****************************************************************************************************************/

/**
 Computes the gradients of the gaussian relative to the given coordinates (x,y), results
 are stored in the given gradient matrix M
 
 \param		data		image of the gaussian
 \param		w,h			width and height of the image
 \param		results		parameters of the gaussian at the previous step
 \param		matrice		gauss matrix
 \param		vettore		gauss vector
 
 */
void computeGradient(double* M, int x , int y, double* results){

	double diff_x = 0.0, diff_y = 0.0, frac_x = 0.0, frac_y = 0.0, sig2x =
	0.0, sig2y = 0.0, dexp = 0.0;
	
	diff_x = x - results[PAR_X];
	diff_y = y - results[PAR_Y];
	sig2x = pow(results[PAR_SX], 2);
	sig2y = pow(results[PAR_SY], 2);
	frac_x = pow(diff_x, 2) / sig2x;
	frac_y = pow(diff_y, 2) / sig2y;
	dexp = exp(frac_x + frac_y);
	
	M[0] = 1 / dexp;
	M[1] =
	(results[PAR_A] * (2 * x - 2 * results[PAR_X])) / (sig2x *
													   dexp);
	M[2] =
	(results[PAR_A] * (2 * y - 2 * results[PAR_Y])) / (sig2y *
													   dexp);
	M[3] =
	(2 * results[PAR_A] * pow(diff_x, 2)) /
	(pow(results[PAR_SX], 3) * dexp);
	M[4] =
	(2 * results[PAR_A] * pow(diff_y, 2)) /
	(pow(results[PAR_SY], 3) * dexp);
	M[5] = x;
	M[6] = y;
	M[7] = 1.0;
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
void procedure(const unsigned char *data, int w, int h, double *results,
			   gsl_matrix_view matrice, gsl_vector_view vettore)
{
	
    int npixels = w * h;
	
    int i = 0, x = 0, y = 0, base = 0;
	
    /* Task over the image, for every pixels its coordinates are
	 determined first then the actual data is compared with prediction
	 and used to construct the gradient matrix M and the vector diff */
    for (i = 0; i < npixels; i++) {
		x = (i + 1) % w;
		
		/** in case of a data parallel computation the worker owns only a portion of rows of the image
		 thus the correct heigth of the examined point is determined using the rank of the worker */
		
#ifdef FARM
		y = ((i + 1) / w);
#endif
		
#ifdef DATA_PARALLEL
		y = ((i + 1) / w) + h * my_rank;
#endif
		
		base = i * DIM_FIT;
		
		diff[i] = data[i] - evaluateGaussian(results, x, y);
		
		computeGradient(M+base,x,y,results);
    }
	
    /* Compute matrix = M'*M */
    gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1.0, &gsl_M.matrix,
				   &gsl_M.matrix, 0.0, &matrice.matrix);
    /* Compute vector = M'*diff */
    gsl_blas_dgemv(CblasTrans, 1.0, &gsl_M.matrix, &gsl_diff.vector, 0.0,
				   &vettore.vector);
	
}

/***************************************************************************************************************
 Initialization of the Fit
 ****************************************************************************************************************/

/**
 Given a image it is analyzed and cropped.
 
 \param		parameter	pathname of the file where the gaussian parameters for the simulation are specified
 \param		fit		array of parameters of the gaussian estimated
 \param		matrix		created image of the gaussian
 \param		cropped		crop of the image of the gaussian
 \param		dimx,dimy	dimension of the crop in the x and y axis
 
 */
void init2(unsigned char *matrix,int width,int height, double *fit)
{
    /* parameters for the cookie cutter */
    int x0, y0, span_x, span_y;
	
    /* parameters for the mask */
    int max = 0, min = 0;
	
    /* pixel mask for reduce the dimension of the region to analyze */
    unsigned char *mask = NULL;
	
    /* writing the image to be fitted in a TIFF file */
#if DEBUG
    writeImage(matrix, (char *) "gaussiana.tiff", width,
			   height);
#endif
    maxmin(matrix, width, height, &max, &min);
	
#if DEBUG
    printf("MAX: %d MIN: %d\n", max, min);
#endif
	
    /* a pixel mask is created in order to reduce the dimensione of the
	 region to analyze with the centroid */
    mask =
	createMask(matrix, width, height, max, min,
			   CROP_PARAMETER);
	
#if DEBUG
    writeImage(mask, (char *) "mask.tiff", width, height);
#endif
	
    centroid(mask, width, height, &x0, &y0, &span_x, &span_y);
	
#if DEBUG
    printf("centro in %d - %d\nCon ampiezza %d e %d\n", x0, y0, span_x,
		   span_y);
#endif
	
    fit[PAR_A] = max;
    fit[PAR_X] = span_x;
    fit[PAR_Y] = span_y;
    fit[PAR_SX] = span_x / 2;
    fit[PAR_SY] = span_y / 2;
	fit[PAR_a] = fit[PAR_b] = 0;
    fit[PAR_c] = min;
	
	if(mask)
		free(mask);
}