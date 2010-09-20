#include "macro.h"

void error (char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

ssize_t Read(int fd, const void *buf, size_t nbyte) {
	ssize_t nread = 0, n;
	do {
		if ((n = read(fd, &((char *) buf)[nread], nbyte - nread)) == -1) {
			/* "EINTR is not really an error */
			if (errno == EINTR)
				continue;
			/* ERROR */
			else
				return -1;
		}
		/* EOF */
		if (n == 0) 
			return nread;
		nread = nread + n;
	} while (nread < nbyte);
	return nread;
}

ssize_t Write(int fd, const void *buf, size_t nbyte) {
	int nwritten = 0, n;
	do {
        	if ((n = write(fd, &((const char *) buf)[nwritten], nbyte - nwritten)) == -1) {
            		if (errno == EINTR)
                		continue;
            		else 
				return -1;	
        	}
        	nwritten = nwritten + n;
    	} while (nwritten < nbyte);
    	return nwritten;
}
