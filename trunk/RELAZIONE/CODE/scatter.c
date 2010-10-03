/** 
\file	scatter.c
\brief	functions which performs scattering of images, and reduce of double matrix.
		MPI_Rank EMITTER and COLLECTOR should be defined as PS, number of service nodes.
 */

#include "scatter.h"
#include "parallel.h"

#include <stdio.h>
#include <stdlib.h>

#define IMAGE 0
#define MATRIX 1

extern int p, my_rank;

/**
 Given an image of dimension dim, represented as a unsigned char matrix.
 It is scattered by EMITTER among the workers.
 
 @param		matrix				image
 @param		dim					dimension of the image
 @param		partition			local buffer of workers
 
 */
void scatterImage(unsigned char *matrix, int dim, unsigned char *partition)
{
    int i, ppw;
    MPI_Status status;

    ppw = dim / (p - PS);

    if (my_rank == EMITTER) {
	for (i = PS; i < p; i++) {
	    MPI_Send(matrix + (i - PS) * ppw, ppw, MPI_UNSIGNED_CHAR, i,
		     IMAGE, MPI_COMM_WORLD);
	}
    } else {
	if (my_rank >= PS) {
	    MPI_Recv(partition, ppw, MPI_UNSIGNED_CHAR, EMITTER, IMAGE,
		     MPI_COMM_WORLD, &status);
	}
    }
}

/**
 Given a scattered double matrix, it is collected by the COLLECTOR
 node using MPI_Recv and it is reduced.
 
 @param		matrix				matrix where reduce is performed
 @param		dim					dimension of double matrix
 @param		patition			partition of the matrix among workers
 
 */
void reduceMatrix(double *matrix, int dim, double *partition)
{
    int i, j;
    double *buffer;
    MPI_Status status;

    buffer = (double *) malloc(sizeof(double) * dim);

    if (my_rank == COLLECTOR) {
	for (j = 0; j < dim; j++) {
	    matrix[j] = 0;
	}
	for (i = PS; i < p; i++) {
	    MPI_Recv(buffer, dim, MPI_DOUBLE, i, MATRIX, MPI_COMM_WORLD,
		     &status);
	    for (j = 0; j < dim; j++) {
		matrix[j] = matrix[j] + buffer[j];
	    }
	}
    } else {
	if (my_rank >= PS) {
	    MPI_Send(partition, dim, MPI_DOUBLE, COLLECTOR, MATRIX,
		     MPI_COMM_WORLD);
	}
    }
    free(buffer);
}
