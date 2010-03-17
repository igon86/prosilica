CC = g++
FLAGS = -DDEBUG -ltiff -D_x86 -D_LINUX -lgsl -lgslcblas -lm

.PHONY : sequenziale test clean

sequenziale:
	$(CC) $(FLAGS) ./sequenziale.cpp ./tiffPostElaboration.cpp ./fit.cpp -o sequenziale

test:
	./sequenziale ./parameters

clean:
	rm -f RESULTS* *.tiff FIT Centroid *~
