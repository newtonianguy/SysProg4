#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <math.h>
#include <errno.h>
#define MAXRCVLEN 500
#define PORTNUM 2343

pthread_attr_t attr;

//function passed to create thread
void *threadFunc(void* arg){//This is what the thread runs

	//declares variables
	int len, *mysocket = (int*)arg, openCheck;
	int sock = *((int*)arg);//converts argument into socket connection
	int counter, part=0, start=0;
	size_t length=0;
	char buffer[MAXRCVLEN];//for reading in message
	char *msg[3];//for deconstructing message into readable commands
	char *mess;//message that gets sent back to client. Will have return value and error value
	char *er;//error value
	char *re;//return value
	//reads in message
	len = recv(mysocket[0], buffer, MAXRCVLEN, 0);
	buffer[len] = '\0';
	
	//reads string client sent
	for(counter=0; counter<len;counter++){
		if(buffer[counter] == '\0'){
			break;
		}
		if(buffer[counter] == 44){//breaks apart message at commas
			if( part>=3 ){//makes sure too many parameters weren't sent by the client
				printf("Error:Too many parameters were sent by the client.\nCommand message chould have three parameters.\nCommas separate paramaters.\n");
			}
			printf("Found break in the message\n");
			msg[part] = (char*) malloc(length+1);
			strncpy(msg[part], &buffer[start], length);
			part++;
			start = counter + 1;
			length = 0;
			continue;
		}
		length++;
	}
	msg[part] = (char*) malloc(length+1);
	strncpy(msg[part], &buffer[start], length);
	
	//checks to see if this is a close message
	if(part==0){
		//calls close and gets return and error messages
		int fd = atoi(msg[0]);
		int ret;
		ret = close(fd);
		if(ret <= 0){
				re = "0";
		}
		else{
				re = (char*) malloc((int)((ceil(log10(ret))+1)*sizeof(char)) + 1);
				sprintf(re, "%d", ret);
		}
		if(errno <= 0){
				er = "0";
		}
		else{
			er = (char*) malloc((int)((ceil(log10(errno))+1)*sizeof(char)));
			sprintf(er, "%d", errno);
		}
		
		//sends back message to client
		mess = (char*) malloc(sizeof(er) + sizeof(re) + 2);
		strcat(mess, re);
		strcat(mess, ",");
		strcat(mess, er);
		strcat(mess, "\0");
		send(sock, mess, strlen(mess), 0);
		return NULL;
	}
	
	//checks to see if command message was sent correctly
	if( part < 2 ){
		printf("Error:Too few parameters were sent by the client.\nCommand message chould have three parameters.\nCommas separate paramaters.\n");
	}
	//interprets message and performs commands
	openCheck = atoi(msg[0]);
	if(openCheck != 0){//this means the message is a "read" or "write" command
		int bytes;
		if(strcmp(msg[1], "read\0") == 0){//checks to see if it is a "read" command
			int size = atoi(msg[2]);//size of read or write
			char buff[size];
			bytes = read(openCheck, buff, size);
			if(bytes <= 0){
				re = "0";
			}
			else{
				re = (char*) malloc((int)((ceil(log10(bytes))+1)*sizeof(char)) + 1);
				sprintf(re, "%d", bytes);
			}
			if(errno <= 0){
				er = "0";
			}
			else{
				er = (char*) malloc((int)((ceil(log10(errno))+1)*sizeof(char)));
				sprintf(er, "%d", errno);
			}
			
			//makes response message
			mess = (char*) malloc(sizeof(er) + sizeof(re) + size + 3);
			strcat(mess, re);
			strcat(mess, ",");
			strcat(mess,buff);
			strcat(mess, ",");
			strcat(mess, er);
			strcat(mess, "\0");
			send(sock, mess, strlen(mess), 0);
		}
		else if(strcmp(msg[1], "write\0") == 0){//checks to see if this is a write command
			int size = strlen(msg[2]);//size of read or write
			bytes = write(openCheck, msg[2], size);
			if(bytes <= 0){
				re = "0";
			}
			else{
				re = (char*) malloc((int)((ceil(log10(bytes))+1)*sizeof(char)) + 1);
				sprintf(re, "%d", bytes);
			}
			if(errno <= 0){
				er = "0";
			}
			else{
				er = (char*) malloc((int)((ceil(log10(errno))+1)*sizeof(char)));
				sprintf(er, "%d", errno);
			}
			//makes response message
			mess = (char*) malloc(sizeof(er) + sizeof(re) + 2);
			strcat(mess, re);
			strcat(mess, ",");
			strcat(mess, er);
			strcat(mess, "\0");
			printf("Here2\n");
			send(sock, mess, strlen(mess), 0);
		}
		else{
			printf("Error:Client did not send an identifiable command to server\n");
			return NULL;
		}
	}
	else{//otherwise the command is an "open" command
		//TODO:Check for incorrect open command message
		int flags = atoi(msg[2]);
		int retur;
		retur = open(msg[1],flags);
		re = (char*) malloc((int)((ceil(log10(retur))+1)*sizeof(char)));
		sprintf(re, "%d", retur);
		if(errno == 0){
			er = "0";
		}
		else{
			er = (char*) malloc((int)((ceil(log10(errno))+1)*sizeof(char)));
			sprintf(er, "%d", errno);
		}
		//makes response message
		mess = (char*) malloc(sizeof(er) + sizeof(re) + 2);
		strcat(mess, re);
		strcat(mess, ",");
		strcat(mess, er);
		strcat(mess, "\0");
		send(sock, mess, strlen(mess), 0);
	}
	return NULL;
}

int main(int argc, char *argv[]){
	//initializes the thread attributes
	pthread_t tnum=0;
	pthread_attr_init(&attr);
	//sets pthread variables

	//sets up server 
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
	pthread_create(&tnum, &attr, &threadFunc, (void*) &conSocket);//creates thread
	tnum++;
	while(conSocket){
		printf("Incoming connection from %s - sending welcome\n",inet_ntoa(dest.sin_addr));
		conSocket = accept(mySocket, (struct sockaddr *)&dest, &socksize);//accepts new connection
		pthread_create(&tnum,&attr,&threadFunc,(void*) &conSocket);//creates thread for new connection
		tnum++;
	}
	pthread_attr_destroy(&attr);//destroys unecesarry attributes
	close(conSocket);
	close(mySocket);
	return EXIT_SUCCESS;
}