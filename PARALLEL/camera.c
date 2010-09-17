#include "camera.h"

/**
 * Main function
 * @param the server address
 */

int main(int argc, char *argv []) {

	int sock, count = 0;
	char buffer [N_BUF];
	struct hostent *server;
	struct sockaddr_in serv_addr;

	if (argc < 2)
		error("Error in the arguments");

	/* open the socket*/
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		error("Error opening socket");

	/* take the IP address from the firts argument of the program */
	if ((server = gethostbyname(argv[1])) == NULL)
		error("Error in Ip address");

	memset((char *) buffer, ZERO, N_BUF);
	memcpy((char *) buffer, "mamma", strlen("mamma"));

	/* set the address and the port of the server */
	memset((char *) &serv_addr, ZERO, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *) &serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
	serv_addr.sin_port = htons(PORT);
    
	/* connect the client to server */
	while (connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		if (errno != ENOENT)
			error("Error connecting");
		sleep(1);
		if (++count == MAX_TRY)	
			error("Unable to contact server");
	}

	/* write and close the socket */
	if(write(sock, buffer, strlen(buffer)) < 0)
		error("Error writing in socket");
	
	if (close(sock) < 0)
		error("Error closing the socket");

	return 0;
}
