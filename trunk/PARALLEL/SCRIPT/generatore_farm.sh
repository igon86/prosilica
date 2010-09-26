#!/bin/bash

#grado di parallelismo della macchina
np=16
x=10
y=10
#grado di parallelismo minimo
#indici
k=1

FILE_FLAG=./FLAG_FARM
FILE_DIM=./DIMENSIONI

num_flag=$(cat $FILE_FLAG | wc -l )
num_dim=$(cat $FILE_DIM | wc -l )
echo "numero di diversi flag: $num_flag"
echo "numero di diverse dimensioni: $num_dim"

#apro il file
echo "echo INIZIO" > test.sh
#diversi flag di compilazione
while [ $k -le $num_flag ]
do
	#mi piglio la riga di parametri
	FLAGS=$(head -n $k $FILE_FLAG | tail -n 1)
	#e il grado di parallelismo minimo scritto nella riga sotto
	k=$[$k+1]
	MIN_P=$(head -n $k $FILE_FLAG | tail -n 1)
	# inizializzo il nome del file
	FLAG_TAGNAME=""
	for j in $FLAGS;
	do
		#compilo il nome per il file in output (i flag sono tutti attaccati)
		FLAG_TAGNAME="$FLAG_TAGNAME$j"
	done
	echo $FLAG_TAGNAME
	
	#COMPILO
	echo "mpicc $FLAGS -DFARM -Wall -pedantic  -I/home/parallel/lottarin/include -lgsl -lgslcblas -lm -ltiff ./farm.c ./fit.c ./image.c ./macro.c -L/home/parallel/lottarin/lib -o farm" >> test.sh

	n=1
	#diverse dimensioni
	while [ $n -le $num_dim ]
	do
		#leggo la dimensione da file
		dim=$(head -n $n $FILE_DIM | tail -n 1)
		#aggiorno il nome del file di output
		TAGNAME="$FLAG_TAGNAME-$dim"
		#echo $TAGNAME
		#diversi gradi di parallelismo
		i=$[$MIN_P]
		while [ $i -le $np ]
		do
			if [ $(echo $TAGNAME | grep MODULE ) ]; then
				echo "mpirun -np $i ./farm $dim $dim >> farm_$TAGNAME &" >> test.sh
				echo "./camera" >> test.sh
			else	
				echo "mpirun -np $i ./farm $dim $dim >> ./TEMPI/farm_$TAGNAME" >> test.sh
			fi
			
			i=$[$i+1]
		done
		
		n=$[$n+1]	
	done
	
	k=$[$k+1]
done