#include "camera.h"

/**
 * Main function
 * @param the server address
 * @param dimx
 * @param dimy
 */
int main(int argc, char *argv []) {

	int sock, count = 0, i = 0, dimx = 0, dimy = 0, dim;
	unsigned char *buffer;

	struct hostent *server;
	struct sockaddr_in serv_addr;

	if (argc < 4)
		error("Error in the arguments");

	/* open the socket*/
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		error("Error opening socket");

	/* take the IP address from the firts argument of the program */
	if ((server = gethostbyname(argv[1])) == NULL)
		error("Error in Ip address");

	/* set the dimension */
	dimx = atoi(argv[2]);
	dimy = atoi(argv[3]);

	dim = dimx * dimy;	

	/* set the address and the port of the server */
	memset((char *) &serv_addr, ZERO, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *) &serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
	serv_addr.sin_port = htons(PORT);
    
	/* connect the client to server */
	while (connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {

		perror("Error connecting");
		
		sleep(1);
		if (++count == MAX_TRY)	
			error("Unable to contact server");
	}

	
	/* send the parameters to server */
	if(Write(sock, &dimx, sizeof(int)) < sizeof(int))
		error("Error writing the integer");
	if(Write(sock, &dimy, sizeof(int)) < sizeof(int))
		error("Error writing the integer");

	for(i = 0; i < STREAMLENGTH; i++){
		/* create the image */
		buffer = createImage(dimx ,dimy);
		/* write and close the socket */
		if(Write(sock, buffer, dim) < dim)
			error("Error writing in socket");
		/* free the memory*/
		free(buffer);
	}
	
	if (close(sock) < 0)
		error("Error closing the socket");

	return 0;
}
