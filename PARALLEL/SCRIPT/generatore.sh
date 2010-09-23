#!/bin/bash

np=16
x=10
y=10
i=1

FLAGS="-o3 -DON_DEMAND -DFARM -DMODULE"

echo "mpicc -D_X86 -D_OSX -mmacosx-version-min=10.4 -Wall -pedantic $FLAGS -I/opt/local/var/macports/software/gsl/1.12_0/opt/local/include -lgsl -lgslcblas -lm -ltiff ./farm.c ./fit.c ./image.c ./macro.c -L/opt/local/var/macports/software/gsl/1.12_0/opt/local/lib -o farm" > test.sh

for j in $FLAGS;
do
TAGNAME="$TAGNAME$j"
done

while [ $i -le $np ]
do
    if [ $(echo $TAGNAME | grep MODULE ) ]; then
	echo "mpirun -np $i ./farm $x $y >> farm_$TAGNAME &" >> test.sh
	echo "./camera" >> test.sh
    else	
	echo "mpirun -np $i ./farm $x $y >> farm_$TAGNAME" >> test.sh
    fi
    i=$[$i+1]
done