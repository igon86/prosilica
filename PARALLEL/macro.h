#ifndef __MACRO__H
#define __MACRO__H

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

void error (char *msg);

ssize_t Read (int fd, const void *buf, size_t nbyte);

ssize_t Write (int fd, const void *buf, size_t nbyte);

#endif
