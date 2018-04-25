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
	if(flags!=O_RDWR && flags!=O_WRONLY && flags!=O_RDONLY){
		errno = 1;
		printf("Error in netopen():You may only pass O_RDWR, O_WRONLY, or O_RDONLY as a flag.\n%s",strerror(errno));
		return -1;
	}
	char buffer[MAXRCVLEN + 1]; //+1 so we can add null terminator
	int len, mysocket, length = 0, counter, ret;
	char *msg;
	
	//converts "flags" from int to char
	char *flag, *re, *er;
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
	close(mysocket);
	//interprets message(commas separate return and error values)	
	for(counter=0; counter<len;counter++){
		if(buffer[counter] == 44){//breaks apart message at commas
			re = (char*) malloc(length+1);//gets return value
			strncpy(re, &buffer[0], length);
			length++;
			break;
		}
		length++;
	}
	//TODO:inform user when an error occurs
	er = (char*) malloc(len - length+1);
	strncpy(er, &buffer[length], (len - length));
	errno = atoi(er);
	ret = atoi(re);
	if(ret == -1){
		printf("Error in netopen():%s\n",strerror(errno));
	}
	return ret;
}

size_t netread(int fildes, void *buf, size_t nbyte){
	if(fildes==-1){
		errno = 9;
		printf("Error in netread():%s\n",strerror(errno));
		return -1;
	}
	char buffer[MAXRCVLEN + 1], *re[2], *er; //+1 so we can add null terminator
	int len, mysocket, counter, length=0, ret, start=0, part=0;
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
	close(mysocket);
	for(counter=0; counter<len;counter++){
		if(buffer[counter] == 44){//breaks apart message at commas
			re[part] = (char*) malloc(length + 1);//gets return value
			strncpy(re[part], &buffer[start], length);
			start = counter + 1;
			part++;
			length = 0;
			continue;
		}
		length++;
	}
	//TODO:inform user when an error occurs
	er = (char*) malloc(length + 1);
	strncpy(er, &buffer[length], (length + 1));
	errno = atoi(er);
	ret = atoi(re[0]);
	strncpy(buf, re[1], nbyte);
	if(ret == -1){
		printf("Error in netread():%s\n",strerror(errno));
	}
	return ret;
}

size_t netwrite(int fildes, void *buf, size_t nbyte){
	if(fildes == -1){
		errno = 9;
		printf("Error in netwrite():%s\n",strerror(errno));
		return -1;
	}
	if(strlen(buf) > nbyte){
		printf("Warning:Buffer longer than size.\nSome of the text will get cut-off.\n");
	}
	char buffer[MAXRCVLEN + 1]; //+1 so we can add null terminator
	int len, mysocket, counter, length=0, ret;
	int fileLen = (int)((ceil(log10(fildes))+1)*sizeof(char));//size of file as a string
	int byteLen = (int)((ceil(log10(nbyte))+1)*sizeof(char));//size of file as a string
	char fd[fileLen],bytes[byteLen], byt[nbyte], *re, *er;
	strncpy(byt, buf, nbyte);

	//converts int params into string for sending to server
	sprintf(fd,"%d",fildes);
	sprintf(bytes,"%d",(int)nbyte);

	//makes message that will be sent to server
	char *msg;
	msg = (char*) malloc(fileLen + nbyte + 8);//7 is for ",read," and null terminator
	strcat(msg, fd);
	strcat(msg, ",write,");
	strcat(msg, byt);
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
	close(mysocket);
	
	//receive return message
	len = recv(mysocket, buffer, MAXRCVLEN, 0);
	// We have to null terminate the received data ourselves 
	buffer[len] = '\0';
	printf("Received %s (%d bytes).\n", buffer, len);
	close(mysocket);
	for(counter=0; counter<len;counter++){
		if(buffer[counter] == 44){//breaks apart message at commas
			re = (char*) malloc(length + 1);//gets return value
			strncpy(re, &buffer[0], length);
			length++;
			break;
		}
		length++;
	}
	//TODO:inform user when an error occurs
	er = (char*) malloc(len - length + 1);
	strncpy(er, &buffer[length], (len - length));
	errno = atoi(er);
	ret = atoi(re);
	if(ret == -1){
		printf("Error in netwrite():%s\n",strerror(errno));
	}
	return ret;
}

int netclose(int fd){
	if(fd == -1){
		errno = 9;
		printf("Error in netclose():%s\n",strerror(errno));
		return -1;
	}
	char buffer[MAXRCVLEN + 1]; //+1 so we can add null terminator
	int len, mysocket, length = 0, counter, ret;
	char *msg, *re, *er;
	
	int fileLen = (int)((ceil(log10(fd))+1)*sizeof(char));//size of file descriptor as a string
	char fild[fileLen];
	
	//makes message that will be sent to server
	msg = (char*) malloc(fileLen+1);
	strcpy(msg, fild);
	
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
	close(mysocket);
	//interprets message(commas separate return and error values)	
	for(counter=0; counter<len;counter++){
		if(buffer[counter] == 44){//breaks apart message at commas
			re = (char*) malloc(length+1);//gets return value
			strncpy(re, &buffer[0], length);
			length++;
			break;
		}
		length++;
	}
	//TODO:inform user when an error occurs
	er = (char*) malloc(len - length + 1);
	strncpy(er, &buffer[length], (len - length));
	errno = atoi(er);
	ret = atoi(re);
	if(ret == -1){
		printf("Error in netclose():%d \n",errno);
	}
	return ret;
}

int main(int argc, char *argv[])
{
	int fd;
	char buf[20];
	fd = netopen("test/test1.txt",O_RDWR);
	//open("test/test1.txt",O_RDWR);
	//x = netclose(fd);
	//printf("%d\n",errno);
	read(fd, buf, 20);
	//char *buff = "You fool! I have been trained in the Jedi arts by Count Dooku.";
	//netwrite(fd, buff, strlen(buff));
	netclose(fd);
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

