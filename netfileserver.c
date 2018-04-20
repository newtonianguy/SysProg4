#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#define MAXRCVLEN 500
#define PORTNUM 2343

pthread_attr_t attr;

//function passed to create thread
void *threadFunc(void* arg){//This is what the thread runs
	int sock = *((int*)arg);//converts argument into socket connection
	int counter, start=0, length=0, parts=0;
	char buffer[MAXRCVLEN + 1];//for reading in message
	char *msg[3];//for deconstructing message into readable commands
	char *mess;//message that gets sent back to client. Will have return value and error value
	char *er;//error value
	char *re;//return value
	//TODO:Read in message from client
	
	//reads string client sent
	for(counter=0; counter<MAXRCVLEN ;counter++){ 
		if(buffer[counter] == 44){//breaks apart message at commas
			if( part>=3 ){//makes sure too many parameters weren't sent by the client
				printf("Error:Too many parameters were sent by the client.\nCommand message chould have three parameters.\nCommas separate paramaters.\n");
			}
			printf("Found break in the message\n");
			msg[part] = (char*) malloc(length+1);
			strcat(msg[part],'\0');//adds null character to end 
			part++;
			start = counter + 1;
		}
		length++;
	}
	//checks to see if command message was sent correctly
	if( part!=3 ){
		printf("Error:Too few parameters were sent by the client.\nCommand message chould have three parameters.\nCommas separate paramaters.\n");
	}
	//interprets message and performs commands
	if(msg[0][0] == 45 || (47 < msg[0][0] && msg[0][0] < 58)){//this means the message is a "read" or "write" command
		int fd = atoi(msg[0]);
		int size = atoi(msg[2]);//size of read or write
		char buff[size];
		
		if(strcmp(msg[1], "read") == 0){//checks to see if it is a "read" command
			read(fd, buff, size);
			//makes response message
			//TODO:Set response and error values
			mess = (char*) malloc(sizeof(er) + sizeof(re) + 1);
			strcat(mess, re);
			strcat(mess, er);
			strcat(mess, '\0');
			send(sock, mess, strlen(msg), 0);
		}
		else if(strcmp(msg[1], "write") == 0){//checks to see if this is a write command
			write(fd, buff, size);
			//makes response message
			//TODO:Set response and error values
			mess = (char*) malloc(sizeof(er) + sizeof(re) + 1);
			strcat(mess, re);
			strcat(mess, er);
			strcat(mess, '\0');
			send(sock, mess, strlen(msg), 0);
		}
		else{
			printf("Error:Client did not send an identifiable command to server\n");
			return;
		}
	}
	else{//otherwise the command is an "open" command
		//TODO:Check for incorrect open command message
		int flags = atoi(msg[2]);
		open(msg[1],msg[2]);
		//makes response message
		//TODO:Set response and error values
		mess = (char*) malloc(sizeof(er) + sizeof(re) + 1);
		strcat(mess,re);
		strcat(mess,er);
		strcat(mess,'\0');
		send(sock, mess, strlen(msg), 0);
	}
	return;
}

int main(int argc, char *argv[]){
	//initializes the thread attributes
	int s,tnum=0;
	s = pthread_attr_init(&attr);
	//sets pthread variables
	
	//sets up server 
	char *msg = "Connection Received\n";
	struct sockaddr_in dest;//socket info about the machine connecting to us
	struct sockaddr_in serv;//socket info about the server
	int mySocket;//socket used for listening for incoming connections
	socklen_t socksize = sizeof(struct sockaddr_in);
	
	memset(&serv, 0, sizeof(serv));//zero the struct before filling the fields
	serv.sin_family = AF_INET;//set the type of connection to TCP/IP
	serv.sin_addr.s_addr = htonl(INADDR_ANY);//set our address to any interface
	serv.sin_port = htons(PORTNUM);//set the server port number
	
	mySocket = socket(AF_INET, SOCK_STREAM, 0);
	
	bind(mySocket, (struct sockaddr *)&serv, sizeof(struct sockaddr));//bind server info to mysocket
	
	//starts listening for connections to queue
	listen(mySocket, 1);
	int conSocket = accept(mySocket, (struct sockaddr *)&dest, &socksize);
	s = pthread_create(tnum,&attr,&threadFunc,conSocket);//creates thread
	
	while(conSocket){
		printf("Incoming connection from %s - sending welcome\n",inet_ntoa(dest.sin_addr));
		conSocket = accept(mySocket, (struct sockaddr *)&dest, &socksize);//accepts new connection
		s = pthread_create(tnum,&attr,&threadFunc,(void*) &conSocket);//creates thread for new connection
	}
	s = pthread_attr_destroy(&attr);//destroys unecesarry attributes
	close(conSocket);
	close(mySocket);
	return EXIT_SUCCESS;
}