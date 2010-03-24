#include "fit.h"

static FILE* fitDebug = fopen("FIT", "w");
static unsigned char *crop = NULL;

/***************************************************************************************************************
 Centroid
 ****************************************************************************************************************/

/** image need to be the output of a createMask function
 both the center position and the dimension of the centroid depends
 on the filter parameter of the createMask function previously used*/

void centroid(unsigned char *image, int w, int h, double *x, double *y, double *sigma_x, double *sigma_y) {	
	int npixels = w * h;
	//support data arrays
	int counth [h];
	int countw [w];
	int count = 0;
	int i = 0;
    	for (i; i < h; i++)
		counth[i] = 0;
    	for (i = 0; i < w; i++)
		countw[i] = 0;
	for (i = 0; i < npixels; i++) {
		unsigned temp = image[i];
		if (temp) {
			++count;
			++countw[i % w];
			++counth[i / w];
		}
    	}
	
	double w_center = 0;
	double h_center = 0;

	int left_border = 0;
	int right_border = 0;
	int down_border = 0;
	int up_border = 0;
	
    //W CENTER
	for (i = 0; i < w; i++) {
		w_center = w_center + ((double) countw[i] / count) * (i + 1);
		if (!left_border && countw[i])
			left_border = i;
		if (left_border && !right_border && !countw[i])
			right_border = i;
	}
	
    //H CENTER
	for (i = 0; i < h; i++) {
		h_center = h_center + ((double) counth[i] / count) * (i + 1);
		if (!down_border && counth[i])
			down_border = i;
		if (down_border && !up_border && !counth[i])
			up_border = i;
	}
	*x = w_center;
	*y = h_center;
	*sigma_x = (right_border - left_border) / 2.0;
	*sigma_y = (up_border - down_border) / 2.0;
}



/***************************************************************************************************************
 Matrix
 ****************************************************************************************************************/
unsigned char * createMatrix (int length, int width, double* result){
	int i = 0, j = 0;	
	int dim = length * width;
	unsigned char* matrix = (unsigned char*) malloc (dim);
	unsigned char* p = matrix;	
	/* build the image */
	for (i = 0;i < length; i++)
		for(j = 0; j < width; j++)
			*p++ = (unsigned char) evaluateGaussian(result, j, i);
	return matrix;
}

/***************************************************************************************************************
 Cookie
 ****************************************************************************************************************/

unsigned char *createMask(unsigned char *image, int w, int h, int max, int min, double filter) {
	int threshold = (int) (filter * (max - min));
	
    	int npixels = w * h;
    //unsigned char *cookie = new unsigned char[npixels];
	unsigned char *cookie = new unsigned char[npixels];
	for (int i = 0; i < npixels; i++) {
		unsigned char temp = image[i];
		if (temp > threshold)
			cookie[i] = MASSIMO;
		else
			cookie[i] = 0;
	}
	return cookie;
}

/***************************************************************************************************************
 Crop function
 ****************************************************************************************************************/

unsigned char *cropImage(const unsigned char *input, int w, int h, int x1, int x2, int y1, int y2)
{
	
    int count = 0;
    int limit = w * (y2 + 1);
    int dimension = (x2 - x1 + 1) * (y2 - y1 + 1);
    if (crop == NULL) {
#if DEBUG
		printf("CROP INITIALIZED!!\n");
#endif
		crop = (unsigned char*) malloc(dimension);
    }
    for (int i = 0; i < limit; i++) {
		int a = i % w;
		int b = i / w;
		if (a >= x1 && a <= x2 && b >= y1 && b <= y2) {
			crop[count++] = input[i];
		}
    }
    return crop;
}

/***************************************************************************************************************
 Evaluate Gaussian at coordinates (x,y)
 ****************************************************************************************************************/

double evaluateGaussian(double* gaussian, int x, int y)
{
#if DEBUG
//printf("Valuto la gaus: %f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", gaussian[PAR_A], gaussian[PAR_X],
//		gaussian[PAR_Y], gaussian[PAR_SX], gaussian[PAR_SY], gaussian[PAR_a], gaussian[PAR_b], gaussian[PAR_c]);
#endif
    double slope = gaussian[PAR_a] * x + gaussian[PAR_b] * y + gaussian[PAR_c];
    double x_arg = pow(((double) x - gaussian[PAR_X]), 2.0) / pow(gaussian[PAR_SX], 2.0);
    double y_arg = pow(((double) y - gaussian[PAR_Y]), 2.0) / pow(gaussian[PAR_SY], 2.0);
    double arg = -(x_arg + y_arg);
    double z = gaussian[PAR_A] * exp(arg) + slope;
    return z;
}

/***************************************************************************************************************
 Iterative NLLS fit algorithm
 ****************************************************************************************************************/

int iteration(const unsigned char *data, int w, int h, double * results)
{

    int npixels = w * h;
    double *diff = new double[npixels];
    int temp;
    int x, y;
    int index;
    gsl_vector *delta = gsl_vector_alloc(8);
    gsl_vector *vettore = gsl_vector_alloc(8);
	
    fprintf(fitDebug, "Done most of it\n");
    fflush(fitDebug);
	
    /** variables used to keep track of the square error*/
    double square;
    int iteration = 0;
    double squares[10];
    for (int i = 0; i < 10; i++)
		squares[i] = 0;
	
    fprintf(fitDebug, "Starting with with algebra stuff..");
    fflush(fitDebug);
	
    int dimension = 8 * npixels;
	
    fprintf(fitDebug, "dimension..");
    fflush(fitDebug);
	
    double *M = new double[dimension];
	
    fprintf(fitDebug, "M..");
    fflush(fitDebug);
	
    double matrix[8 * 8];
	
    fprintf(fitDebug, "matrix..");
    fflush(fitDebug);
	
    double vector[8];
	
    fprintf(fitDebug, "Done with algebra stuff\n");
    fflush(fitDebug);
	
    double initial_error = 0;
	
    double diff_x, diff_y;
    double frac_x, frac_y, sig2x, sig2y, dexp;
    int base;
    double test;
	
    fprintf(fitDebug, "Done with inizialization\n");
    fflush(fitDebug);
	
    /**ITERATION LOOP */
    while (iteration < 1) {
		/*
		 printf("LOOP %d\n",iteration+1);
		 fflush(stdout);
		 */
		
		/** ITERATIVE PROCEDURE OVER THE IMAGE*/
		fprintf(fitDebug, "Iteration: %d\n", iteration);
		fflush(fitDebug);
		for (int i = 0; i < npixels; i++) {
			
			x = (i + 1) % w;
			y = (i + 1) / w;
			
			base = i * 8;
			test = evaluateGaussian(results, x, y);
			diff[i] = data[i] - test;
			
			
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
			//derivative of the slopePlan !
			M[base + 5] = x;
			M[base + 6] = y;
			M[base + 7] = 1.0;
			

		}
		
		square = 0.0;
		/* square calculation and array adjustment */
		for (int index1 = 0; index1 < npixels; index1++) {
			square = square + pow(diff[index1], 2);
		}
		squares[iteration] = square;
		
		iteration++;
		
		/**initial error print*/
		if (iteration == 1) {
			initial_error = square;
			fprintf(fitDebug, "ERRORE INIZIA: %09.0f\n", initial_error);
			fflush(fitDebug);
		}
		fflush(stdout);
		
		/**
		 FILE* fp = fopen("matrice.mat","w");
		 for (int index1 = 0;index1<500;index1++){
		 for (int index2=0;index2<8;index2++){
		 fprintf(fp,"%08f\t",M[index1*8+index2]);
		 }
		 fprintf(fp,"\n");
		 }
		 */
		
		gsl_matrix_view gsl_M = gsl_matrix_view_array(M, npixels, 8);
		gsl_matrix_view matrice = gsl_matrix_view_array(matrix, 8, 8);
		
		gsl_vector_view differenze = gsl_vector_view_array(diff, npixels);
		
		//printf("\nMM\n");
		
		/* Compute matrix = M'*M */
		gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1.0, &gsl_M.matrix, &gsl_M.matrix, 0.0, &matrice.matrix);
		/**
		 for (int index1 = 0;index1<8;index1++){
		 for (int index2 =0;index2<8;index2++){
		 //				printf("%8f\t",matrix[index1*8 + index2]);
		 }
		 //			printf("\n");
		 }
		 */
		/** Compute matrix = M'*M
		 printf("MM2\n");
		 for (int index1 = 0;index1<8;index1++){
		 for(int index2 = 0;index2<8;index2++){
		 matrix[index1*8 + index2] = 0;
		 for (int index3 = 0;index3<npixels;index3++){
		 matrix[index1*8 + index2] = matrix[index1*8 + index2] + M[index3*8+index1]*M[index3*8+index2];
		 }
		 }
		 }
		 for (int index1 = 0;index1<8;index1++){
		 for (int index2 =0;index2<8;index2++){
		 printf("%08.2f\t",matrix[index1*8 + index2]);
		 }
		 printf("\n");
		 }
		 */
		
		//printf("\nVECTOR\n");
		
		/* Compute vector = M'*diff for (int i =8; i<16 ;i++){ for (int
		 k=0;k<npixels;k++){ vector[i] = vector[i] + M[k*8 + i]*diff[k];
		 } } for (int index1 = 0;index1<8;index1++){
		 printf("%08.2f\t",vector[index1]); } printf("\n"); */
		/* Compute vector = M'*diff */
		gsl_blas_dgemv(CblasTrans, 1.0, &gsl_M.matrix, &differenze.vector, 0.0, vettore);
		
		//gsl_vector_fprintf(stdout, vettore, "%g");
		
		//printf("\n");
		
		/* Compute the delta vector of deviation */
		
		
		int s;
		
		gsl_permutation *p = gsl_permutation_alloc(8);
		
		gsl_linalg_LU_decomp(&matrice.matrix, p, &s);
		
		gsl_linalg_LU_solve(&matrice.matrix, p, vettore, delta);
		//printf("delta = \n");
		//gsl_vector_fprintf(stdout, delta, "%g");
		
		/** result adjustment */
		results[PAR_A]  = results[PAR_A]  + gsl_vector_get(delta, 0);
		results[PAR_X]  = results[PAR_X]  + gsl_vector_get(delta, 1);
		results[PAR_Y]  = results[PAR_Y]  + gsl_vector_get(delta, 2);
		results[PAR_SX] = results[PAR_SX] + gsl_vector_get(delta, 3);
		results[PAR_SY] = results[PAR_SY] + gsl_vector_get(delta, 4);
		results[PAR_a]  = results[PAR_a]  + gsl_vector_get(delta, 5);
		results[PAR_b]  = results[PAR_b]  + gsl_vector_get(delta, 6);
		results[PAR_c]  = results[PAR_c]  + gsl_vector_get(delta, 7);
		
		
		//RIPROVA ! !!
	    //printf("New x_0 is %f\n", results->x_0);
    }
	
#if DEBUG
    fprintf(fitDebug, "ERRORE FINALE: %09.0f\n", square);
    fprintf(fitDebug, "STRUCT FINALE: \n");
    fflush(fitDebug);
#endif
	
    /* FREE!!!!!!!! */
    if (M)
		delete M;
    if (diff)
		delete diff;
    gsl_vector_free(vettore);
    gsl_vector_free(delta);
	
	
	
    if (square > initial_error) {
		return 0;
    } else {
		return 1;
    }
}

/***************************************************************************************************************
 Calculate Max & Min of an image
 ****************************************************************************************************************/

void maxmin(unsigned char *image, int w, int h, int *max, int *min)
{
	
    int npixels = w * h;
    *max = 0;
    *min = 255;
    unsigned char temp;
	
    for (int i = 0; i < npixels; i++) {
		temp = image[i];
		if (temp > *max)
			*max = temp;
		if (temp < *min)
			*min = temp;
    }
}

/***************************************************************************************************************
				Writing mono8 black and white tiff function
****************************************************************************************************************/

void writeImage(unsigned char* image,char* dest, int w, int h){
		
		TIFF* out = TIFFOpen(dest, "w");
		
		//8bit image
		int sampleperpixel = 1;
		
		TIFFSetField (out, TIFFTAG_IMAGEWIDTH, w);  // set the width of the image
		TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);    // set the height of the image
		TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);   // set number of channels per pixel
		TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);    // set the size of the channels
		TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
		//   Some other essential fields to set that you do not have to understand for now.
		TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

		//a quanto pare si scrive una row alla volta!!! quindi una row e` lunga:
		tsize_t linebytes = sampleperpixel * w;     // length in memory of one row of pixel in the image.
		
		// buffer used to store the row of pixel information for writing to file
		unsigned char *buf = (unsigned char*) NULL;        
		//    Allocating memory to store the pixels of current row
		if (TIFFScanlineSize(out) == linebytes)
			buf =(unsigned char *)_TIFFmalloc(linebytes);
		else
			buf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(out));
		
		
		// We set the strip size of the file to be size of one row of pixels
		TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, w*sampleperpixel));
		
		//Now writing image to the file one strip at a time
		for (uint32 row = 0; row < h; row++)
		{
			//printf("%d ",row);
			//fflush(stdout);
			memcpy(buf, &image[(h-row-1)*linebytes], linebytes);    // check the index here, and figure out why not using h*linebytes
			//printf("memcpy ");
			//fflush(stdout);
			if (TIFFWriteScanline(out, buf, row, 0) < 0) break;
			//printf("writeline ");
			//fflush(stdout);
		}
		(void) TIFFClose(out);
		if (buf) _TIFFfree(buf);
}

