#include "tiffPostElaboration.hpp"

/***************************************************************************************************************
						MAIN
****************************************************************************************************************/

int main(int argc, char *argv[]){
	TIFF* tif = TIFFOpen(argv[1],"r");
	double filter;
	if(argc == 3) filter = atof(argv[2]);
	else filter = 0.5;
	printf("APERTA\n");
	//width and height of the picture
	if(tif){
		//modw is just w-1 is inizialited and used only for noise reduction
		uint32 w, h,modw;
		uint16 p,s,photometric,o;
		size_t npixels;
		uint32* raster;
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
		printf("sto analizzando un immagine %dx%d\n",w,h);
		npixels = w * h;
		raster = (uint32*) _TIFFmalloc(npixels * sizeof (uint32));
		TIFFGetField(tif,TIFFTAG_BITSPERSAMPLE, &p);
		TIFFGetFieldDefaulted(tif,TIFFTAG_SAMPLESPERPIXEL, &s);
		TIFFGetFieldDefaulted(tif,TIFFTAG_ORIENTATION,&o);
		printf("L'immagine ha %d sample per pixel ",s);
		printf("ognuno %d bit per sample\n",p);
		if(o == 1)
			printf("immagine etero: %d\n",o);
		else
			printf("immagine curiosa: %d\n",o);
		TIFFGetField(tif,TIFFTAG_PHOTOMETRIC,&photometric);
		printf("tipo photometrico:%d\n",photometric);
		fflush(stdout);
		unsigned char *image;
		unsigned char *immagine;
		int max = 0;
		int min = 255;
		if (raster != NULL) {
			//COPIA IMMAGINE IN CHAR[]
			immagine=new unsigned char [w*h*s];
			if (TIFFReadRGBAImage(tif, w, h, raster, 0)) {
				unsigned char R;
				unsigned char G;
				unsigned char B;
				unsigned char A;
				for (int i=0;i<npixels;i++){
					R=(char)TIFFGetR(raster[i]);
					G=(char)TIFFGetG(raster[i]);
					B=(char)TIFFGetB(raster[i]);
					if(R > max){
						max = R;
					}
					if(R < min){
						min = R;
					}
					immagine[i]= (unsigned char)R;
				}
				printf("Test terminato: max=%d min=%d\n",max,min);		
			}
			//CHAR[] PER LA MASK
			image=new unsigned char [w*h*s];
			unsigned char temp;
			int threshold = (int) filter*max;
			for(int i=0;i<npixels;i++){
				unsigned char temp = (unsigned char)TIFFGetR(raster[i]);
				if(temp>threshold) image[i]=255;
				else image[i]=0; //basta dire poti poti. me le puoi toccare ma non dire poti poti se no mammina sa cosa vuol dire poti poti e sa cosa stai facendo! lo sa lo sa. e ovvio amore. 
			}
			_TIFFfree(raster);
		}
		//creo l'immagine per la maschera e ci copio la vecchia
		TIFF* out = TIFFOpen("mask.tiff", "w");
		// valori per inizializzare la nuova immagine
		int width = w;
		int height = h;
		int sampleperpixel = s;
		
		TIFFSetField (out, TIFFTAG_IMAGEWIDTH, width);  // set the width of the image
		TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);    // set the height of the image
		TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);   // set number of channels per pixel
		TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);    // set the size of the channels
		TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
		//   Some other essential fields to set that you do not have to understand for now.
		TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

		//a quanto pare si scrive una row alla volta!!! quindi una row e` lunga:
		tsize_t linebytes = sampleperpixel * width;     // length in memory of one row of pixel in the image.
		
		// buffer used to store the row of pixel information for writing to file
		unsigned char *buf = (unsigned char*) NULL;        
		//    Allocating memory to store the pixels of current row
		if (TIFFScanlineSize(out) == linebytes)
			buf =(unsigned char *)_TIFFmalloc(linebytes);
		else
			buf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(out));
		
		
		// We set the strip size of the file to be size of one row of pixels
		TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, width*sampleperpixel));
		
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
		if (image) delete(image);
		
		/****************************************************************************************************************************/
		//pulizia
		printf("vuoi pulire i pixel?");
		int c = getchar();
		if (c == 's'){
			printf("NOISE REDUCTION!!!\n");
			modw = w-1;
			tif = TIFFOpen("mask.tiff","r");
			//rifaccio un nuovo raster
			raster = (uint32*) _TIFFmalloc(npixels * sizeof (uint32));
			//rifaccio una nuova image in cui copiare dal raster
			image=new unsigned char [w*h*s];
			if (raster != NULL) {
				if (TIFFReadRGBAImage(tif, w, h, raster, 0)) {
					//copio tutto nella image
					int flag = 0;
					/**
					non lo uso piu`. metto solo la stamap del cookie
					for(int i=0;i<npixels;i++){
						unsigned temp = (unsigned char)TIFFGetR(raster[i]);
						//checko se e` acceso -> uso un flaggettino ciccioloso per rendere piu` veloce il check
						if(temp){
							int mod = i%w;
							if(mod==0 || mod==modw){
								image[i]=0;
							}
							else{
								//a questo punto devo checkare se ha dei vicini o e` un pixel morto -> da rifare
								if (flag || TIFFGetR(raster[(i-1)%npixels]) || TIFFGetR(raster[(i+1)%npixels])){
									image[i]=255;
								}
							}
							flag = 1;
						}
						else{
							flag = 0;
							image[i]=0;
						}
					}	
					//else image[i]=0; //basta dire poti poti. me le puoi toccare ma non dire poti poti se no mammina sa cosa vuol dire poti poti e sa cosa stai facendo! lo sa lo sa. e ovvio amore.
					*/ 
					for(int i=0;i<npixels;i++){
						unsigned temp = (unsigned char)TIFFGetR(raster[i]);
						//checko se e` acceso -> uso un flaggettino ciccioloso per rendere piu` veloce il check
						if(temp){
							image[i]=immagine[i];
						}
						else{
							image[i]=0;
						}
					}	

				}
				//COPIO IMMAGINE SU TIFF!! pari pari a quella vecchia
				out = TIFFOpen("maskclean.tiff", "w");
				
				TIFFSetField (out, TIFFTAG_IMAGEWIDTH, width);  // set the width of the image
				TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);    // set the height of the image
				TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);   // set number of channels per pixel
				TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);    // set the size of the channels
				TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
				//   Some other essential fields to set that you do not have to understand for now.
				TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
				TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
				
				printf("Dovrebbe aver settato tutti i parametri\nESCO!!\n");
				fflush(stdout);
				//a quanto pare si scrive una row alla volta!!! quindi una row e` lunga:
				
				// buffer used to store the row of pixel information for writing to file
				unsigned char *buf = (unsigned char*) NULL;        
				//    Allocating memory to store the pixels of current row -> da capire
				if (TIFFScanlineSize(out) == linebytes)
					buf =(unsigned char *)_TIFFmalloc(linebytes);
				else
					buf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(out));
				
				
				// We set the strip size of the file to be size of one row of pixels
				TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, width*sampleperpixel));
				
				printf("sto per iniziare il ciclo\n");
				fflush(stdout);
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
				if (image) delete(image);
				
			}
			
		}
		
		/****************************************************************************************************************************/		
		printf("CENTRE & SPOT SIZE DETECTION!!\n");
		tif = TIFFOpen("mask.tiff","r");
		//rifaccio un nuovo raster
		raster = (uint32*) _TIFFmalloc(npixels * sizeof (uint32));
		//rifaccio una nuova image in cui copiare dal raster
		image=new unsigned char [w*h*s];
		
		if (raster != NULL) {
			if (TIFFReadRGBAImage(tif, w, h, raster, 0)) {
				//creo gli array di supporto in cui salvare il numero di pixel trovati
				int counth[h];
				int countw[w];
				int count=0;
				for (int i=0;i<h;i++) counth[i]=0;
				for (int i=0;i<w;i++) countw[i]=0;
				for(int i=0;i<npixels;i++){
					unsigned temp = (unsigned char)TIFFGetR(raster[i]);
					if(temp){
						//printf("1");
						++count;
						++countw[i%w];
						++counth[i/w];
					}
					//else printf("0");
				}	
				printf("Ho beccato %d pixel\n",count);
				//else image[i]=0; //basta dire poti poti. me le puoi toccare ma non dire poti poti se no mammina sa cosa vuol dire poti poti e sa cosa stai facendo! lo sa lo sa. e ovvio amore. 
				
				//CENTRO W
				double w_center = 0;
				double h_center = 0;
				
				for (int i=0;i<w;i++){
					w_center = w_center + ((double) countw[i] / count)*(i+1);
					//printf("%d: %f ",i,media);
				}
				
				//CENTRO H
				for (int i=0;i<h;i++){
					h_center = h_center + ((double) counth[i] / count)*(i+1);
				}
				
				printf("Centro in %f,%f\n",w_center,h-h_center);
				if((int) TIFFGetR(raster[((int)(h-h_center))*w])) printf("TODO BIEN");
				//gli sputo la riga del centro per OCTAVE
				//RILEGGO ANCORA LA MALEDETTA IMMAGINE ORIGINALE, questo codice fa schifo!!
				tif = TIFFOpen(argv[1],"r");
				TIFFReadRGBAImage(tif, w, h, raster, 0);
				int i= (int) (h_center)*w;
				unsigned char data[w];
				int founded=0;
				for (int j=0;j<w;j++){
					data[j] = (unsigned char)TIFFGetR(raster[i+j]);
				}
				octaveVectorPort(data,w);
				octaveMatrixPort(immagine,w,h);
				matlabMatrixPort(immagine,w,h);
			}
			if (buf) _TIFFfree(buf);
			if (image) delete(image);
			
		}
		
	}
}
