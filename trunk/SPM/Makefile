CC = g++
FLAGS = -DDEBUG -ltiff -D_x86 -D_LINUX -lgsl -lgslcblas -lm

.PHONY : sequenziale tests

sequenziale:
	$(CC) $(FLAGS) ./sequenziale.cpp ./tiffPostElaboration.cpp ./fit.cpp -o sequenziale

tests:
	./sequenziale ./parameters
