#ifndef __PARALLEL__H
#define __PARALLEL__H

#include <mpi.h>
#include <sys/time.h>

/* length of the stream */
#define STREAMLENGTH 1

/* rank emettitor */
#define EMITTER 0

/* rank collector */
#define COLLECTOR 1
	
/* number of service processes */
#define PS 2

/* define MPI tags of the messages */
enum {
	PARAMETERS = 0,
	IMAGE,
	RESULTS,
	REQUEST,
	TERMINATION
};

#endif
