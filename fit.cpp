#include "fit.hpp"
#include "tiffPostElaboration.hpp"

FILE* centroidDebug = fopen("Centroid","w");
FILE* fitDebug = fopen("FIT","w");
FILE* risultati = fopen("RESULTS","w");

static unsigned char* crop=NULL;

/***************************************************************************************************************
							Cookie cutter initialization
****************************************************************************************************************/

static void cropInitialize(int dimension){
	crop = new unsigned char[dimension];
}

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

	int npixels = w*h;
	unsigned char* cookie = new unsigned char [npixels];
	for(int i=0;i<npixels;i++){
		unsigned char temp = image[i];
		if(temp > threshold) cookie[i]=MAX;
		else cookie[i]=0; 
	}
	return cookie;
}

/***************************************************************************************************************
					Crop function
****************************************************************************************************************/

unsigned char* cropImage(const unsigned char *input, int w,int h,int x1,int x2,int y1,int y2){

#if DEBUG
	fprintf(fitDebug,"CROP: %d - %d   %d - %d su immagine %dx%d\n",x1,x2,y1,y2,w,h);
	fflush(stdout);
#endif

	int count = 0;
	int limit = w*(y2+1);
	int dimension = (x2-x1+1)*(y2-y1+1);
	if(crop == NULL){
		printf("CROP INITIALIZED!!\n");
		cropInitialize(dimension);
	}
	for (int i = 0;i<limit;i++){
		int a = i%w;
		int b = i/w;
		if(a >= x1 && a <= x2 && b >= y1 && b <= y2){
			crop[count++] = input[i];
		}
	}
	return crop;
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
					Print fit struct
****************************************************************************************************************/

void printFit(FILE* fp,fit_t* f){

	fprintf(fp,"A: %f\nx_0: %f\ty_0: %f\nsigma_x: %f\tsigma_y: %f\na: %f\tb: %f\tc: %f\n",f->A,f->x_0,f->y_0,f->sigma_x,f->sigma_y,f->a,f->b,f->c);
}

/***************************************************************************************************************
					Print fit struct
****************************************************************************************************************/

void printResults(fit_t* f){

	fprintf(risultati,"%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",f->A,f->x_0,f->y_0,f->sigma_x,f->sigma_y,f->a,f->b,f->c);
	fflush(risultati);
}

/***************************************************************************************************************
		Create M'M matrix -> this is temporary and should be replaced with BLAS function calls
****************************************************************************************************************/

/***************************************************************************************************************
		Create M'diff vector -> this is temporary and should be replaced with BLAS function calls
****************************************************************************************************************/

/***************************************************************************************************************
					Iterative NLLS fit algorithm
****************************************************************************************************************/
int iteration(const unsigned char* data,int w,int h,fit_t* results){
	
	fprintf(fitDebug,"IMMAGINE: %dx%d \nSTARTING STRUCT: \n",w,h);
	printFit(fitDebug,results);
	fflush(fitDebug);
	
	int npixels = w*h;
	double* diff = new double [npixels];
	double min = 255;
	double max = -255;
	int temp;
	int x,y;
	int index;
	gsl_vector *delta = gsl_vector_alloc (8);
	gsl_vector *vettore = gsl_vector_alloc (8);
	
	fprintf(fitDebug,"Done most of it\n");
	fflush(fitDebug);
	
	/** variables used to keep track of the square error*/
	double square;
	int iteration = 0;
	double squares[10];
	for (int i=0;i<10;i++) squares[i] = 0; 
	
	fprintf(fitDebug,"Starting with algebra stuff..");
	fflush(fitDebug);

	int dimension = 8*npixels;
	
	fprintf(fitDebug,"dimension..");
	fflush(fitDebug);
	
	double* M = new double[dimension];
	
	fprintf(fitDebug,"M..");
	fflush(fitDebug);
	
	double matrix[8*8];
	
		fprintf(fitDebug,"matrix..");
	fflush(fitDebug);
	
	double vector[8];
	
	fprintf(fitDebug,"Done with algebra stuff\n");
	fflush(fitDebug);
	
	double initial_error=0;
	
	double diff_x,diff_y;
	double frac_x,frac_y,sig2x,sig2y,dexp;
	int base;
	double test;
		
	fprintf(fitDebug,"Done with inizialization\n");
	fflush(fitDebug);
	
	/**ITERATION LOOP */
	while (iteration < 1){
		/*
		printf("LOOP %d\n",iteration+1);
		fflush(stdout);
		*/
		
		/* fake inizialization to follow the weird MATLAB PATTERN TOPLEFT-COLUMNWISE-STARTING DOWN*/
		int row = h;
		int column=1;
		/** ITERATIVE PROCEDURE OVER THE IMAGE*/
		fprintf(fitDebug,"Iteration: %d\n",iteration);
		fflush(fitDebug);
		for (int i=0; i<npixels; i++){
			
			//WEIRD!!!!  I'M USING THE ROWS AS OFFSET TO IMPLEMENT THE COLUMWISE FASHION
			x = (i+1)%h;
			y = (i+1)/h;
			
			//WEIRD INDEX CALCULATION
			int index = (row-1)*w +(column-1);
			
			base = i*8;
			test = evaluateGaussian(results,x,y);
			diff[i] = data[index] - test;
			
			
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
			M[base+3] = (2*results->A*pow(diff_x,2)) /  (pow(results->sigma_x,3) * dexp );
			M[base+4] = (2*results->A*pow(diff_y,2)) /  (pow(results->sigma_y,3) * dexp );
			//derivative of the slopePlan!
			M[base+5] = x;
			M[base+6] = y;
			M[base+7] = 1.0;
			
			//WEIRD CIRCULAR INCREMENT
			row--;
			if(row == 0){
				row = h;
				column++;
			}
		}
		
		square=0.0;
		/* square calculation and array adjustment */ 
			for (int index1=0; index1<npixels;index1++){
				square = square + pow(diff[index1],2);
			}
			squares[iteration] = square;
		
			iteration++;
			
			/**initial error print*/
			if(iteration == 1){
				initial_error = square;
				fprintf(fitDebug,"ERRORE INIZIA: %09.0f\n",initial_error);
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
		gsl_matrix_view matrice = gsl_matrix_view_array(matrix,8, 8);
		
		gsl_vector_view differenze = gsl_vector_view_array(diff, npixels);
		
		//printf("\nMM\n");
		
		/* Compute matrix = M'*M */
		gsl_blas_dgemm (CblasTrans, CblasNoTrans,1.0, &gsl_M.matrix, &gsl_M.matrix,0.0,&matrice.matrix);
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
		gsl_blas_dgemv (CblasTrans, 1.0,&gsl_M.matrix, &differenze.vector, 0.0,vettore);
		
		//gsl_vector_fprintf (stdout, vettore, "%g");
		
		//printf("\n");
		
		/* Compute the delta vector of deviation */
		
		
		int s;
		
		gsl_permutation * p = gsl_permutation_alloc (8);
		
		gsl_linalg_LU_decomp (&matrice.matrix, p, &s);
		
		gsl_linalg_LU_solve (&matrice.matrix, p, vettore, delta);
		//printf ("delta = \n");
		//gsl_vector_fprintf (stdout, delta, "%g");
		
		/** result adjustment */
		results->A = results->A + gsl_vector_get(delta,0);
		//printf("New A is: %f\n",results->A);
		
		//ADDING
		results->x_0 = results->x_0 + gsl_vector_get(delta,1);
		results->y_0 = results->y_0 + gsl_vector_get(delta,2);
		results->sigma_x = results->sigma_x + gsl_vector_get(delta,3);
		results->sigma_y = results->sigma_y + gsl_vector_get(delta,4);
		results->a = results->a + gsl_vector_get(delta,5);
		results->b = results->b + gsl_vector_get(delta,6);
		results->c = results->c + + gsl_vector_get(delta,7);
		
		
		//RIPROVA!!!
		//printf("New x_0 is %f\n",results->x_0);
	}
	
#if DEBUG	
	fprintf(fitDebug,"ERRORE FINALE: %09.0f\n",square);
	fprintf(fitDebug,"STRUCT FINALE: \n");
	printFit(fitDebug,results);
	fflush(fitDebug);
#endif
	//printResults(results);
	
	/*FREE!!!!!!!!*/
	if(M) delete M;
	if(diff) delete diff;
	gsl_vector_free(vettore);
	gsl_vector_free(delta);
	
	
	
	if(square>initial_error){
		return 0;
	}
	else{
		return 1;
	}
}

/***************************************************************************************************************
							Calculate Max & Min of an image
****************************************************************************************************************/

void maxmin(const unsigned char* image,int w,int h,int* max,int* min){

	int npixels = w*h;
	*max = 0;
	*min = 255;
	unsigned char temp;
	
	for(int i =0 ;i<npixels; i++){
		temp = image[i];
		if(temp > *max) *max = temp;
		if(temp < *min) *min = temp;
	}
}
