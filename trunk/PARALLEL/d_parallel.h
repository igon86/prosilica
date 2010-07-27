#ifndef __PARALLEL__H
#define __PARALLEL__H

#include <mpi.h>
#include <sys/time.h>

/* length of the stream */
#define STREAMLENGTH 4

/* rank emettitor */
#define EMITTER 0

#define PS 1       


/* macro for define MPI tags of the messages */
#define PARAMETERS 0
#define IMAGE 1
#define RESULTS 2
#define REQUEST 3
#define TERMINATION 4

#endif
