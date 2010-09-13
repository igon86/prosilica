include ../conf

CC = mpicc
FLAGS = -Wall -pedantic -DDEBUG -DON_DEMAND
GSL_LIB = -lgsl -lgslcblas
LIB = $(GSL_LIB) -lm -ltiff

NP=6
P=4
WIDTH=300
HEIGHT=200

.PHONY : farm test_farm dataparallel test_dataparallel clean valgrind regression backup

farm:
	$(CC)$(CVER) -D_$(ARCH) -D_$(OS) $(FLAGS) -DFARM $(GSL_INCLUDE_DIR) $(LIB) ./parallel.c ./fit.c ./image.c $(GSL_LIB_DIR) -o farm

dataparallel:
	$(CC)$(CVER) -D_$(ARCH) -D_$(OS) $(FLAGS) -DDATA_PARALLEL $(GSL_INCLUDE_DIR) $(LIB) ./data_parallel.c ./fit.c ./image.c $(GSL_LIB_DIR) -o dataparallel

test_farm:
	mpirun -np $(NP) ./farm $(WIDTH) $(HEIGHT)

test_dataparallel:
	mpirun -np $(P) ./dataparallel $(WIDTH) $(HEIGHT)

valgrind:
	$(CC)$(CVER) -D_$(ARCH) -D_$(OS) $(FLAGS) -g $(GSL_INCLUDE_DIR) $(LIB) ./valgtester.c ./fit.c $(GSL_LIB_DIR) -o valgtest
	valgrind --leak-check=full --show-reachable=yes ./valgtest ./parameters

regression: farm test_farm dataparallel test_dataparallel

backup:
	cp *.c ./BACKUP

clean:
	rm -f RESULTS* *.tiff FIT Centroid *~ farm dataparallel valgtest *.o *.BAK
