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
	int mySocket;
	
}