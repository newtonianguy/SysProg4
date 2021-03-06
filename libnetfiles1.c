//TODO:do stuff
//make netopen(), netread(), netwrite(), abd netclose()
#include "libnetfiles.h"

//char *Umode;

int netserverinit(const char* hostname, const char* mode){
	//Umode = mode; 
	if( gethostbyname(hostname) == NULL ){
		printf("Error: Host does not exist\n");
		return -1;
	}
	printf("Host does exist.\n");
	return 0;
}

int netopen(const char *pathname, int flags){
	char buffer[MAXRCVLEN + 1]; //+1 so we can add null terminator
	int len, mysocket;
	char *msg;
	//converts "flags" from int to char
	char *flag;
	int flagLen = (int)((ceil(log10(flags))+1)*sizeof(char));//size of file as a string
	flag = (char*) malloc(flagLen);
	sprintf(flag,"%d",flags);
	//makes message that will be sent to server
	msg = (char*) malloc(12+strlen(pathname)+flags);
	
	strcat(msg, "exclusive,");
	strcat(msg, pathname);
	strcat(msg, ",");
	strcat(msg, flag);
	strcat(msg, "\0");
	//makes connection
	struct sockaddr_in dest;
	mysocket = socket(AF_INET, SOCK_STREAM, 0);
	memset(&dest, 0, sizeof(dest)); // zero the struct
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr("127.0.0.1"); // set destination IP number
	
	dest.sin_port = htons(PORTNUM); // set destination port number
	
	connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
	//send message
	send(mysocket, msg, strlen(msg), 0);
	//receive return message
	len = recv(mysocket, buffer, MAXRCVLEN, 0);
	// We have to null terminate the received data ourselves 
	buffer[len] = '\0';
	printf("Received %s (%d bytes).\n", buffer, len);
	//TODO:Read and interpret return message
	close(mysocket);
	return 0;
}

size_t netread(int fildes, void *buf, size_t nbyte){
	char buffer[MAXRCVLEN + 1]; //+1 so we can add null terminator
	int len, mysocket;
	int fileLen = (int)((ceil(log10(fildes))+1)*sizeof(char));//size of file as a string
	int byteLen = (int)((ceil(log10(nbyte))+1)*sizeof(char));//size of file as a string
	char fd[fileLen],bytes[byteLen];
	//converts int params into string for sending to server
	sprintf(fd,"%d",fildes);
	sprintf(bytes,"%d",(int)nbyte);
	//makes message that will be sent to server
	char *msg;
	msg = (char*) malloc(fileLen + byteLen + 7);//7 is for ",read," and null terminator
	strcat(msg, fd);
	strcat(msg, ",read,");
	strcat(msg, bytes);
	strcat(msg, "\0");
	//makes connection
	struct sockaddr_in dest;
	mysocket = socket(AF_INET, SOCK_STREAM, 0);
	memset(&dest, 0, sizeof(dest)); // zero the struct
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr("127.0.0.1"); // set destination IP number
	
	dest.sin_port = htons(PORTNUM); // set destination port number
	
	connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
	//send message
	send(mysocket, msg, strlen(msg), 0);
	//receive return message
	len = recv(mysocket, buffer, MAXRCVLEN, 0);
	// We have to null terminate the received data ourselves 
	buffer[len] = '\0';
	printf("Received %s (%d bytes).\n", buffer, len);
	//TODO:Read and interpret return message
	close(mysocket);
	return 0;
}

size_t netwrite(int fildes, void *buf, size_t nbyte){
	char buffer[MAXRCVLEN + 1]; //+1 so we can add null terminator
	int len, mysocket;
	int fileLen = (int)((ceil(log10(fildes))+1)*sizeof(char));//size of file as a string
	int byteLen = (int)((ceil(log10(nbyte))+1)*sizeof(char));//size of file as a string
	char fd[fileLen],bytes[byteLen];
	//converts int params into string for sending to server
	sprintf(fd,"%d",fildes);
	sprintf(bytes,"%d",(int)nbyte);
	//makes message that will be sent to server
	char *msg;
	msg = (char*) malloc(fileLen + byteLen + 7);//7 is for ",read," and null terminator
	strcat(msg, fd);
	strcat(msg, ",write,");
	strcat(msg, bytes);
	strcat(msg, "\0");
	//makes connection
	struct sockaddr_in dest;
	mysocket = socket(AF_INET, SOCK_STREAM, 0);
	memset(&dest, 0, sizeof(dest)); // zero the struct
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr("127.0.0.1"); // set destination IP number
	
	dest.sin_port = htons(PORTNUM); // set destination port number
	
	connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
	//send message
	printf("Hi\n");
	send(mysocket, msg, strlen(msg), 0);
	//receive return message
	len = recv(mysocket, buffer, MAXRCVLEN, 0);
	// We have to null terminate the received data ourselves 
	buffer[len] = '\0';
	printf("Received %s (%d bytes).\n", buffer, len);
	//TODO:Read and interpret return message
	close(mysocket);
	return 0;
}

int netclose(int fd){
	return 0;
}

int main(int argc, char *argv[])
{
	netopen("test/test.txt",O_RDWR);
	/*
	netserverinit("mam1010.ls.rutgers.edu","exclusive");
	char buffer[MAXRCVLEN + 1]; // +1 so we can add null terminator
	int len, mysocket;
	struct sockaddr_in dest;
	mysocket = socket(AF_INET, SOCK_STREAM, 0);
	memset(&dest, 0, sizeof(dest)); // zero the struct 
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr("127.0.0.1"); // set destination IP number
	dest.sin_port = htons(PORTNUM); // set destination port number
	connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
	len = recv(mysocket, buffer, MAXRCVLEN, 0);
	// We have to null terminate the received data ourselves
	buffer[len] = '\0';
	printf("Received %s (%d bytes).\n", buffer, len);
	close(mysocket);
	return EXIT_SUCCESS;
	*/
	return 0;
}

