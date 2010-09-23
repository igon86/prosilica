#!/bin/bash

np=16
dim=2
ITER=5000
shape=nine
i=1
limit=28

echo "gcc -lm -w -DSIMULATORE -DUSE_MPI -DTEST ./generatore_double.c ./M_debug.c ./ctree.c ./codice.c ./algebric.c ./section.c ./crea_albero.c -o generatore" > test.sh
echo "rm generato.c" >> test.sh
echo "cp funzione_"$shape".h funzione.h" >> test.sh       

while [ $i -le $limit ]
do

up=$[$limit-$i+1]
lato=$(tail -n $up ./dimensioni2d_ott | head -n 1 )
echo $lato

echo "./generatore $dim 33 ./shape_$shape $np $ITER" >> test.sh
echo "mpicc -O3 -lm -w -DGENERATO generato.c ./supporto.c ./algebric.c -o worker -lmpich" >> test.sh
echo "mpirun -np $np -machinefile ../m_file ./worker >>LOG_Ottavina_"$shape"_$ITER">> test.sh
echo "tutti.pl kill -9 -1" >> test.sh
echo "rm generato.c" >> test.sh
echo "./generatore $dim 33 ./shape_"$shape"2 $np $ITER" >> test.sh
echo "mpicc -O3 -lm -w -DGENERATO generato.c supporto.c algebric.c -o worker -lmpich" >> test.sh
echo "mpirun -np $np -machinefile ../m_file ./worker >>LOG_Ottavina_"$shape"2_$ITER">> test.sh
echo "tutti.pl kill -9 -1" >> test.sh
echo "rm generato.c" >> test.sh
i=$[$i+1]

done
