#include "camera.h"

int main(int argc, char *argv[])
{
	int sock, new_sock;
	char buffer [N_BUF];
	struct sockaddr_in serv_addr;

	/* server socket */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		error("Error opening socket");

	memset((char *) buffer, ZERO, N_BUF);
	memset((char *) &serv_addr, ZERO, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT);

	if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error("Error on binding");
	
	/* server is listening */
	if (listen(sock, 0) < 0)
		error("Error on listen");

	/* accept the new connection */
	if ((new_sock = accept(sock, NULL, 0)) < 0)
		error("Error on accept");
	
	/* read from the socket */
	if (read(new_sock, buffer, N_BUF) < 0)
		error("Error reading from socket");

	printf("The message is %s\n", buffer);

	/* close the server socket and the socket */
	if (close(new_sock) < 0)
		error("Error closing the server socket");
	if (close(sock) < 0)
		error("Error closing the socket");

	return 0; 
}
