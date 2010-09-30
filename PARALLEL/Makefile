include ../conf

CC = mpicc
FLAGS = -Wall -g -pedantic -DDEBUG
GSL_LIB = -lgsl -lgslcblas
LIB = $(GSL_LIB) -lm -ltiff

NP=4

WIDTH=300
HEIGHT=200

# ON_DEMAND 	MODULE
FARM_FLAGS = $(FLAGS) -DON_DEMAND

# PADDED	ALL	MODULE 			
DP_FLAGS = $(FLAGS) -DMODULE -DPADDED -DALL

.PHONY : farm test_farm dataparallel test_dataparallel clean valgrind regression backup camera

farm:
	$(CC)$(CVER) -D_$(ARCH) -D_$(OS) $(FARM_FLAGS) -DFARM $(GSL_INCLUDE_DIR) $(LIB) ./farm.c ./fit.c ./image.c ./macro.c $(GSL_LIB_DIR) -o farm

dataparallel:
	$(CC)$(CVER) -D_$(ARCH) -D_$(OS) $(DP_FLAGS) -DDATA_PARALLEL $(GSL_INCLUDE_DIR) $(LIB) ./data_parallel.c ./fit.c ./image.c ./macro.c $(GSL_LIB_DIR) -o dataparallel

old_dp:
	$(CC)$(CVER) -D_$(ARCH) -D_$(OS) $(DP_FLAGS) -DFARM $(GSL_INCLUDE_DIR) $(LIB) ./old_dp.c ./fit.c ./image.c ./scatter.c ./macro.c $(GSL_LIB_DIR) -o dataparallel

test_farm:
	mpirun -np $(NP) ./farm $(WIDTH) $(HEIGHT)

test_dataparallel:
	mpirun -np $(NP) ./dataparallel $(WIDTH) $(HEIGHT)

camera:
	gcc $(GSL_INCLUDE_DIR) $(LIB) ./camera.c ./image.c ./fit.c  ./macro.c $(GSL_LIB_DIR) -o camera

regression: farm test_farm dataparallel test_dataparallel old_dp
	mpirun -np $(NP) ./dataparallel $(WIDTH) $(HEIGHT)

backup:
	cp *.c ./BACKUP

clean:
	rm -f RESULTS* *.tiff FIT Centroid *~ farm dataparallel camera valgtest *.o *.BAK
