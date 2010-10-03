#ifndef __SCATTER__H
#define __SCATTER__H

void scatterImage(unsigned char *matrix, int dim,
		  unsigned char *partition);

void reduceMatrix(double *matrix, int dim, double *partition);

#endif
