#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORTNUM 2343

int main(int argc, char *argv[]){
	struct sockaddr_in dest;//socket info about the machine connecting to us
	struct sockaddr_in serv;//socket info about the server
	int mySocket;//socket used for listening for incoming connections
	socklen_t socksize = sizeof(struct sockaddr_in);
	
	memset(&serv, 0, sizeof(serv));//zero the struct before filling the fields
	serv.sin_family = AF_INET;//set the type of connection to TCP/IP
	serv.sinaddr.s_addr = htonl(INADDR_ANY);//set our address to any interface
	serv.sin_port = htons(PORTNUM);//set the server port number
	
	mysocket = socket(AF_INET, SOCKSTREAM, 0);
	
	bind(mysocket, (struct sockaddr *)&serv, sizeof(struct sockaddr));//bind server info to mysocket
	
	//starts listening for connections to queue
	listen(mysocket, 3);
	int conSocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
	
	while(consocket){
		printf("Incoming connection from %s - sending welcome\n",inet_ntoa(dest.sin_addr));
		send(consocket, msg, strlen(msg), 0);
		consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
	}
	close(consocket);
	close(mysocket);
	return EXIT_SUCCESS;
}