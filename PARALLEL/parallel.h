#ifndef __PARALLEL__H
#define __PARALLEL__H

#include <mpi.h>
#include <sys/time.h>

/* length of the stream */
#define STREAMLENGTH 40

/* rank emettitor */
#define EMITTER 0

#ifdef FARM
#define COLLECTOR 1
#endif

/* number of service processes */
#ifdef FARM
#define PS 2
#endif

#ifdef DATA_PARALLEL
#ifdef PADDED
#define PS 1
#else
#define PS 0
#endif
#endif

/* define MPI tags of the messages */
enum {
	PARAMETERS = 0,
	IMAGE,
	RESULTS,
	REQUEST,
	TERMINATION
};

#endif
