

#include "tiffPostElaboration.hpp"

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
				Writing mono8 black and white tiff function
****************************************************************************************************************/

bool writeImage(unsigned char* image,char* dest, int w, int h){
	//creo l'immagine per la maschera e ci copio la vecchia
		TIFF* out = TIFFOpen(dest, "w");
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
		for (uint32 row = 0; row < (uint32) h; row++)
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
		if (image) delete(image);
		return true;
}

