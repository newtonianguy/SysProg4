//TODO:do stuff
//make netopen(), netread(), netwrite(), abd netclose()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#define MAXRCVLEN 500
#define PORTNUM 2343
#include "libnetfiles.h"



int netserverinit(const char* hostname, const char* mode){
	if( gethostbyname(hostname) != 0 ){
		return 0;
	}
	return -1;
}

int netopen(const char *pathname, int flags){
	
	return 0;
}

size_t netread(int fildes, void *buf, size_t nbyte){
	return 0;
}

int netclose(int fd){
	return 0;
}


int main(int argc, char *argv[])
{
	char buffer[MAXRCVLEN + 1]; /* +1 so we can add null terminator */
	int len, mysocket;
	struct sockaddr_in dest;
	mysocket = socket(AF_INET, SOCK_STREAM, 0);
	memset(&dest, 0, sizeof(dest)); /* zero the struct */
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr("127.0.0.1"); /* set destination IP number
	*/
	dest.sin_port = htons(PORTNUM); /* set destination port number
	*/
	connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
	len = recv(mysocket, buffer, MAXRCVLEN, 0);
	/* We have to null terminate the received data ourselves */
	buffer[len] = '\0';
	printf("Received %s (%d bytes).\n", buffer, len);
	close(mysocket);
	return EXIT_SUCCESS;
}

