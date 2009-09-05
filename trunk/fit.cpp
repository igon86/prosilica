
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
	//printf("CROP: %d - %d   %d - %d\nsu immagine %dx%d\n",x1,x2,y1,y2,w,h);
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
	printf("GGG %.2f - %.2f - %.2f - %.2f ",x_arg,y_arg,arg,z);
	return z;
}

/***************************************************************************************************************
					Iterative NLLS fit algorithm
****************************************************************************************************************/
void iteration(const unsigned char* data,int w,int h,fit_t* results){
	int npixels = w*h;

	double* diff = new double [npixels];
	double min = 255;
	double max = -255;
	double temp;

	for (int i =0; i < npixels; i++){
		diff[i] = data[i] - evaluateGaussian(results,i%w,i/w);
		temp = diff[i];
		if(temp > max) max = temp;
		if(temp<min) min = temp;
	}
	printf("Errore massimo: %f \n Errore minimo: %f",max,min);	
	// display the residuals
	//writeImage(diff,"diff.tiff",w,h);
}

