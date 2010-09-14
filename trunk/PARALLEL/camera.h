#ifndef __CAMERA__H
#define __CAMERA__H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>

#define PORT 9999
#define N_BUF 100
#define MAX_TRY 3

void error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

#endif
