#ifndef __PARALLEL__H
#define __PARALLEL__H

#include <mpi.h>
#include <sys/time.h>

/* length of the stream */
#define STREAMLENGTH 4

/* rank emettitor */
#define EMETTITOR 0

/* rank collector */
#if MASTER
	#define COLLECTOR 0
#else
	#define COLLECTOR 1
#endif
	
/* number of service processes */
#if MASTER
	#define PS 1       
#else
	#define PS 2
#endif

/* macro for define MPI tags of the messages */
#define PARAMETERS 0
#define IMAGE 1
#define RESULTS 2
#define REQUEST 3
#define TERMINATION 4

#endif
