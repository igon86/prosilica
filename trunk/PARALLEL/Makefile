CC = mpic++
FLAGS = -DDEBUG -ltiff -D_x86 -D_LINUX -lgsl -lgslcblas -lm

.PHONY : parallel test clean

parallel:
	$(CC) $(FLAGS) ./parallel.cpp ./tiffPostElaboration.cpp ./fit.cpp -o parallel

test:
	./parallel ./parameters

clean:
	rm -f RESULTS* *.tiff FIT Centroid *~
