CC = g++-4.0
FLAGS = -DDEBUG -ltiff -mmacosx-version-min=10.4 -fno-strict-aliasing -fexceptions -I/usr/include -D_x86 -D_OSX -I/opt/local/var/macports/software/gsl/1.12_0/opt/local/include/ -lgsl -lgslcblas -L/opt/local/var/macports/software/gsl/1.12_0/opt/local/lib -lm


.PHONY : sequenziale tests

sequenziale:
	$(CC) $(FLAGS) ./sequenziale.cpp ./tiffPostElaboration.cpp ./fit.cpp -o sequenziale

tests:
	./sequenziale ./parameters