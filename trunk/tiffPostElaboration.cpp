#include "tiffPostElaboration.hpp"

#define MAX 255

/***************************************************************************************************************
				Helper function for math programs
****************************************************************************************************************/

void octaveVectorPort(unsigned char *data,int length){
	FILE *fp = fopen("port.mat","w");
	unsigned char temp[length+1];
	printf("Ho fatto un array di %d caratteri\n",length+1);
	for (int i =0;i<length;i++){
		temp[i]=data[i];
	}
	temp[length]='\0';
	//printf("%s\n",temp);
	fprintf(fp,"# Created by Octave 3.0.0, Tue Aug 04 14:14:09 2009 PDT \n<andrealottarini@igonbook.local>\n# name: Cpport\n# type: matrix\n# rows: 1\n# columns: %d\n",length);
	for (int i=0;i<length;i++){
		fprintf(fp," %d",data[i]);
	}
	fprintf(fp,"\n");
	fclose(fp);
}

void octaveMatrixPort(unsigned char *data,int width,int length){
	FILE *fp = fopen("mport.mat","w");
	printf("faccio la matrice\n");
	fflush(stdout);
	fprintf(fp,"# Created by Octave 3.0.0, Tue Aug 04 14:14:09 2009 PDT \n<andrealottarini@igonbook.local>\n# name: mport\n# type: matrix\n# rows: %d\n# columns: %d\n",length,width);
	for (int i=0;i<length;i++){
		for (int j=0;j<width;j++){
			fprintf(fp," %d",data[i*width+j]);
		}
		fprintf(fp,"\n");
	}
	fclose(fp);
}

void matlabMatrixPort(unsigned char *data,int width,int length){
	FILE *fp = fopen("matrix.mat","w");
	for (int i=0;i<length;i++){
		for (int j=0;j<width;j++){
			fprintf(fp," %d",data[i*width+j]);
		}
		fprintf(fp,"\n");
	}
	fclose(fp);
}

/***************************************************************************************************************
									Centroid
****************************************************************************************************************/

void centroid(unsigned char* image,int w,int h,double* x,double* y){
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
	
	//W CENTER
	for (int i=0;i<w;i++){
		w_center = w_center + ((double) countw[i] / count)*(i+1);
		//printf("%d: %f ",i,media);
	}
	
	//H CENTER
	for (int i=0;i<h;i++){
		h_center = h_center + ((double) counth[i] / count)*(i+1);
	}
	
	*x = w_center;
	*y = h_center;
}

/***************************************************************************************************************
									Cookie
****************************************************************************************************************/

unsigned char* createMask(unsigned char* image,int w,int h,int max,double filter){
	int threshold = filter*max;
	printf("Il limite e` a %d pixel\n",threshold);
	fflush(stdout);
	int npixels = w*h;
	printf("npixel e` %d\n",npixels);
	unsigned char* cookie = new unsigned char [npixels];
	for(int i=0;i<npixels;i++){
		unsigned char temp = image[i];
		if(temp > threshold) cookie[i]=MAX;
		else cookie[i]=0; //basta dire poti poti. me le puoi toccare ma non dire poti poti se no mammina sa cosa vuol dire poti poti e sa cosa stai facendo! lo sa lo sa. e ovvio amore. 
	}
	return cookie;
}

/***************************************************************************************************************
							Read TIFF function
****************************************************************************************************************/
unsigned char* readTIFF(int* width,int* height,int *max,char* link){
	TIFF* tif = TIFFOpen(link,"r");

	uint32 w, h;
	size_t npixels;
	uint32* raster;
	
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
	
	npixels = w * h;
	raster = (uint32*) _TIFFmalloc(npixels * sizeof (uint32));
	
	unsigned char *immagine=new unsigned char[npixels];
	*max = 0;
	
	if (raster != NULL) {

		// filling the image char[]
		unsigned char R;
		if (TIFFReadRGBAImage(tif, w, h, raster, 0)) {
			
			for (int i=0;i<npixels;i++){
				R= (char)TIFFGetR(raster[i]);
				immagine[i]= (unsigned char)R;
				if(R>*max) *max =R;
			}

			_TIFFfree(raster);

		}
		
	}
	*width = w;
	*height = h;
	return immagine;
	
}
/***************************************************************************************************************
				Writing mono8 black and white tiff function
****************************************************************************************************************/

void writeImage(unsigned char* image,char* dest, int w, int h){
		printf("STO PER APRIRE\n");
		fflush(stdout);
		TIFF* out = TIFFOpen(dest, "w");
		printf("APERTA IN SCRITTURA\n");
		fflush(stdout);
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

/***************************************************************************************************************
												Crop function
****************************************************************************************************************/

void cropImage(const unsigned char *input, int w,int h,unsigned char *result,int x1,int x2,int y1,int y2){
	int count = 0;
	int limit = w*y2;
	for (int i = 0;i<limit;i++){
		int a = i%h;
		int b = i*h;
		if(a >= x1 && a <= x2 && b >= y1 && b <= y2) result[count++] = input[i];
	}
}
