#include "parallel.h"
#include "image.h"

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
void writeImage(unsigned char *image, char *dest, int w, int h)
{
	
    TIFF *out = TIFFOpen(dest, "w");
    tsize_t linebytes;
    uint32 row = 0;
    /* buffer used to store the row of pixel information for writing to
	 file */
    unsigned char *buf = NULL;
    /* 8bit image */
    int sampleperpixel = 1;
	
    TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);	/* set the width of the
	 image */
    TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);	/* set the height of the
	 image */
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);	/* set number of
	 channels per pixel */
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);	/* set the size of the
	 channels */
    TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);	/* set the origin of the
	 image */
	
    /* Some other essential fields to set */
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	
    /* determining the dimension of a single row */
    linebytes = sampleperpixel * w;	/* length in memory of one row of
	 pixel in the image */
	
    /* Allocating memory to store the pixels of current row */
    if (TIFFScanlineSize(out) == linebytes)
		buf = (unsigned char *) _TIFFmalloc(linebytes);
    else
		buf = (unsigned char *) _TIFFmalloc(TIFFScanlineSize(out));
	
    /* Set the strip size of the file to be size of one row of pixels */
    TIFFSetField(out, TIFFTAG_ROWSPERSTRIP,
				 TIFFDefaultStripSize(out, w * sampleperpixel));
	
    /* Writing image to the file one strip at a time */
    for (row = 0; row < h; row++) {
		memcpy(buf, &image[(h - row - 1) * linebytes], linebytes);	/* tricky index */
		if (TIFFWriteScanline(out, buf, row, 0) < 0)
			break;
    }
    (void) TIFFClose(out);
    if (buf)
		_TIFFfree(buf);
}


/***************************************************************************************************************
 Create Matrix
 ****************************************************************************************************************/

/**
 Return the image of the gaussian, dimension of the image are given by ( length, width ) parameters while
 parameters of the gaussian are stored in the array input.
 
 \param	height,width		dimensions of the returned image
 \param	input				array containing gaussian parameters
 
 \retval					representation of the gaussian as a 8bit image (unsigned char)
 */
unsigned char *createMatrix(int height, int width, double *input)
{
    int i = 0, j = 0;
    int dim = width * height;
    unsigned char *matrix = (unsigned char *) malloc(dim);
    unsigned char *p = matrix;
#if DEBUG
	printf("Sono nella createMatrix e ho allocato %d\nLA GAUSSIANA RICHIESTA HA I SEGUENTI PARAMETRI:\n",dim);
	for (i=0;i<DIM_FIT;i++) printf("%f\t",input[i]);
	printf("\n");
#endif	
    /* build the image */
    for (i = 0; i < height; i++)
		for (j = 0; j < width; j++)
			*p++ = (unsigned char) evaluateGaussian(input, j, i);
    return matrix;
}

/***************************************************************************************************************
 Create Gaussian
 ****************************************************************************************************************/

/**
 Given a specified width and length it return a representation of the gaussian
 
 \param		dimx,dimy	dimensions of the image
 \retval	representation of the image as a unsigned char matrix 
 */
unsigned char* createImage(int width, int height){
	
    double input[DIM_FIT];
	int i,max;
	
	for(i=0;i<DIM_FIT;i++) input[i] = 0;
	
	
	/* random amplitude value with mean AVERAGE
	 and a maximum deviation of DEVIATION */
	max = AVERAGE + ( rand() % DEVIATION ) - DEVIATION/2;
#if DEBUG
	printf("Max is %d\n",max);
#endif
		
	/* values of the gaussian are chosen with respect of the dimension
	of the image in order to have a nice centered gaussian */ 	
	input[PAR_A] = max;
	input[PAR_X] = width/2;
	input[PAR_Y] = height/2;
	input[PAR_SX] = width/4;
	input[PAR_SY] = height/4;
	
    /* image representing the Gaussian fit */
    return createMatrix(height, width, input);
}
