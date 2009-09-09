#include "fit.hpp"
#include "tiffPostElaboration.hpp"

/***************************************************************************************************************
									Centroid
****************************************************************************************************************/

/** image need to be the output of a createMask function
both the center position and the dimension of the centroid depends 
on the filter parameter of the createMask function previously used*/

void centroid(unsigned char* image,int w,int h,double* x,double* y,double* sigma_x,double* sigma_y){
	int npixels = w*h;

	//support data arrays
	int counth[h];
	int countw[w];
	int count=0;
	for (int i=0;i<h;i++) counth[i]=0;
	for (int i=0;i<w;i++) countw[i]=0;
	for(int i=0;i<npixels;i++){
		unsigned temp = image[i];
		if(temp){
			++count;
			++countw[i%w];
			++counth[i/w];
		}
	}
	
	double w_center = 0;
	double h_center = 0;

	int left_border=0;
	int right_border=0;
	int down_border=0;
	int up_border=0;
	
	//W CENTER
	for (int i=0;i<w;i++){
		w_center = w_center + ((double) countw[i] / count)*(i+1);
		if(!left_border && countw[i]) left_border = i;
		if(left_border && !right_border && !countw[i]) right_border = i;
	}
	
	//H CENTER
	for (int i=0;i<h;i++){
		h_center = h_center + ((double) counth[i] / count)*(i+1);
		if(!down_border && counth[i]) down_border = i;
		if(down_border && !up_border && !counth[i]) up_border = i;
	}
	*x = w_center;
	*y = h_center;
	*sigma_x = (right_border - left_border)/2.0;
	*sigma_y = (up_border - down_border)/2.0;
}

/***************************************************************************************************************
									Cookie
****************************************************************************************************************/

unsigned char* createMask(unsigned char* image,int w,int h,int max,int min,double filter){
	int threshold = (int) (filter*(max-min));
	//printf("CENTROID: Il limite e` a %d pixel\n",threshold);
	fflush(stdout);
	int npixels = w*h;
	unsigned char* cookie = new unsigned char [npixels];
	for(int i=0;i<npixels;i++){
		unsigned char temp = image[i];
		if(temp > threshold) cookie[i]=MAX;
		else cookie[i]=0; //basta dire poti poti. me le puoi toccare ma non dire poti poti se no mammina sa cosa vuol dire poti poti e sa cosa stai facendo! lo sa lo sa. e ovvio amore. 
	}
	return cookie;
}

/***************************************************************************************************************
					Crop function
****************************************************************************************************************/

unsigned char* cropImage(const unsigned char *input, int w,int h,int x1,int x2,int y1,int y2){
	printf("CROP: %d - %d   %d - %d\nsu immagine %dx%d\n",x1,x2,y1,y2,w,h);
	fflush(stdout);
	int count = 0;
	int limit = w*(y2+1);
	int dimension = (x2-x1+1)*(y2-y1+1);
	unsigned char * result = new unsigned char [dimension];
	for (int i = 0;i<limit;i++){
		int a = i%w;
		int b = i/w;
		if(a >= x1 && a <= x2 && b >= y1 && b <= y2){
			result[count++] = input[i];
		}
	}
	return result;
}

/***************************************************************************************************************
				Evaluate Gaussian at coordinates (x,y)
****************************************************************************************************************/

double evaluateGaussian(fit_t* gaussian,int x, int y){
	double slope = gaussian->a*x +gaussian->b*y + gaussian->c;
	double x_arg = pow(((double)x - gaussian->x_0),2.0) / pow(gaussian->sigma_x,2.0);
	double y_arg = pow(((double)y - gaussian->y_0),2.0) / pow(gaussian->sigma_y,2.0);
	double arg = - ( x_arg +  y_arg);
	double z = gaussian->A * exp( arg ) + slope;
	//printf("GGG %.2f - %.2f - %.2f - %.2f ",x_arg,y_arg,arg,z);
	return z;
}

/***************************************************************************************************************
					Iterative NLLS fit algorithm
****************************************************************************************************************/
void iteration(const unsigned char* data,int w,int h,fit_t* results){
	int npixels = w*h;

	double* diff = new double [npixels];
	unsigned char* toPrint = new unsigned char[npixels];
	double min = 255;
	double max = -255;
	int temp;
	int x,y;

	for (int i =0; i < npixels; i++){
		x = i%w;
		y = i/w;
		diff[i] = data[i] - evaluateGaussian(results,x,y);
		temp = (int)diff[i];
		toPrint[i] = abs(temp);
		if(temp > max) max = temp;
		if(temp<min) min = temp;
	}
	printf("Errore massimo: %f \nErrore minimo: %f\n\n",max,min);	
	// display the residuals
	writeImage(toPrint,"diff.tiff",w,h);
	
	//filling the M matrix
	int lda,ldb,ldc;
	lda=8;
	ldb=8;
	ldc=8;
	
	/* consider that M is M' in reality*/
	int dimension = 8*npixels;
	
	double M[dimension];
	double matrix[8*8];
	double vector[8];
	
	
	
	double diff_x,diff_y;
	double frac_x,frac_y,sig2x,sig2y,dexp;
	int base;
	
	for (int i=0; i<npixels; i++){
		x = i%w;
		y = i/w;
		
		base = i*8;
		 
		diff_x = x - results->x_0;
		diff_y = y - results->y_0;
		sig2x = pow(results->sigma_x,2);
		sig2y = pow(results->sigma_y,2);
		frac_x = pow(diff_x,2)/sig2x;
		frac_y = pow(diff_y,2)/sig2y;
		dexp = exp(frac_x + frac_y);
						
		M[base] = 1/dexp;
		M[base+1] = (results->A*(2*x - 2*results->x_0)) / (sig2x*dexp);
		M[base+2] = (results->A*(2*y - 2*results->y_0)) / (sig2y*dexp);
		M[base+3] = (2*results->A*pow(diff_x,2)) / (pow(results->sigma_x,3)*dexp);
		M[base+4] = (2*results->A*pow(diff_y,2)) / (pow(results->sigma_y,3)*dexp);
		//derivative of the slopePlan!
		M[base+5] = x;
		M[base+6] = y;
		M[base+7] = 1.0;
		
	}
	printf("TEST\n");
	for (int index1 = 8;index1<16;index1++){
		printf("%08f\t",M[index1]);
	}
	
	gsl_matrix_view gsl_M = gsl_matrix_view_array(M, npixels, 8);
	gsl_matrix_view matrice = gsl_matrix_view_array(matrix,8, 8);
	
	gsl_vector_view gsl_vector = gsl_vector_view_array(vector, 8);
	
	printf("\nMM\n");

	/* Compute matrix = M'*M */
	gsl_blas_dgemm (CblasTrans, CblasNoTrans,1.0, &gsl_M.matrix, &gsl_M.matrix,0.0,&matrice.matrix);
		
	for (int index1 = 0;index1<8;index1++){
		for (int index2 =0;index2<8;index2++){
			printf("%08.2f\t",matrix[index1*8 + index2]);
		}
		printf("\n");
	}
	
	/* Compute matrix = M'*M */
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
	printf("\nDIFF\n");
	/* Compute vector = M'*diff 
	for (int i =8; i<16 ;i++){
		for (int k=0;k<npixels;k++){
			vector[i] = vector[i] + M[k*8 + i]*diff[k];
		}
	}
	for (int index1 = 0;index1<8;index1++){
		printf("%08.2f\t",vector[index1]);
	}
	printf("\n");
	*/
	/* Compute vector = M'*diff */
	cblas_dgemv (CblasRowMajor, CblasTrans , 8, npixels, 1.0,(double*) &M, lda ,(double*) &diff, 0.0, 0.0,(double*) &vector, 0.0);
	for (int index1 = 0;index1<8;index1++){
		printf("%08.2f\t",vector[index1]);
	}
	printf("\n");
	/* Compute the delta vectorof deviation */
	gsl_matrix_view m = gsl_matrix_view_array ((double* ) matrix, 8, 8);	
	gsl_vector_view b = gsl_vector_view_array (vector, 8);
	
	gsl_vector *cazzo = gsl_vector_alloc (8);
	
	int s;
     
	gsl_permutation * p = gsl_permutation_alloc (8);
     
	gsl_linalg_LU_decomp (&m.matrix, p, &s);
     
	//gsl_linalg_LU_solve (&m.matrix, p, &b.vector, delta);
	printf ("delta = \n");
	//gsl_vector_fprintf (stdout, delta, "%g");
}

