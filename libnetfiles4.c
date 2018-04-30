//TODO:do stuff
//make netopen(), netread(), netwrite(), abd netclose()
#include "libnetfiles.h"

const char *Umode;
int mysocket1 = -1;

int EISDIRcheck(const char *pathname){
	struct stat path_stat;
	stat(pathname, &path_stat);
	return S_ISREG(path_stat.st_mode);
}


int netserverinit(const char* hostname, const char* mode){
	Umode = mode; 
	
	//checks to see if the hostname exists
	if( gethostbyname(hostname) == NULL ){
		printf("Error: HOST_NOT_FOUND\n");
		errno = 22;
		mysocket1 = -1;
		return -1;
	}
	
	//checks the mode that was passed
	if(strcmp(mode, "unrestricted")!=0 && strcmp(mode, "exclusive")!=0 && strcmp(mode, "transaction")!=0){
		printf("Error:INVALID_FILE_MODE.You passed %s.\nOnly unrestricted, exclusive, and transaction.\n",mode);
		errno = 22;
		mysocket1 = -1;
		return -1;
	}

	//makes connection
	struct sockaddr_in dest;
	mysocket1 = socket(AF_INET, SOCK_STREAM, 0);
	memset(&dest, 0, sizeof(dest)); // zero the struct
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr("127.0.0.1"); // set destination IP number
	dest.sin_port = htons(PORTNUM); // set destination port number
	connect(mysocket1, (struct sockaddr *)&dest, sizeof(struct sockaddr));
	
	return 0;
}

int netopen(const char *pathname, int flags){
	//checks connection
	if(mysocket1 == -1){
		printf("Error:Socket connection was never made\n");
		
		return -1;
	}
	
	//checks flags
	if(flags!=O_RDWR && flags!=O_WRONLY && flags!=O_RDONLY){
		errno = 1;
		printf("Error in netopen():You may only pass O_RDWR, O_WRONLY, or O_RDONLY as a flag.\n%s",strerror(errno));
		return -1;
	}
	/*
	//error EISDIR, check if filename is directory
	if(EISDIRcheck(pathname)){
		errno = 1;
		printf("Error in netopen():You may only open a file, not a directory.\n%s",strerror(errno));
		return -1;
	}
	*/
	char buffer[MAXRCVLEN + 1]; //+1 so we can add null terminator
	int len, length = 0, counter, ret;
	char *msg;
	
	//converts "flags" from int to char
	char *flag, *re, *er;
	int flagLen = (int)((ceil(log10(flags))+1)*sizeof(char));//size of file as a string
	flag = (char*) malloc(flagLen);
	sprintf(flag,"%d",flags);
	
	//makes message that will be sent to server
	msg = (char*) malloc(2+strlen(pathname)+strlen(Umode)+flags);
	strcat(msg, Umode);
	strcat(msg, ",");
	strcat(msg, pathname);
	strcat(msg, ",");
	strcat(msg, flag);
	strcat(msg, "\0");
	
	/*
	//makes connection
	struct sockaddr_in dest;
	mysocket = socket(AF_INET, SOCK_STREAM, 0);
	memset(&dest, 0, sizeof(dest)); // zero the struct
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr("127.0.0.1"); // set destination IP number
	dest.sin_port = htons(PORTNUM); // set destination port number
	connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
	*/
	
	//send message
	send(mysocket1, msg, strlen(msg), 0);

	//receive return message
	len = recv(mysocket1, buffer, MAXRCVLEN, 0);
	// We have to null terminate the received data ourselves 
	buffer[len] = '\0';
	printf("Received %s (%d bytes).\n", buffer, len);
	close(mysocket1);
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
	
	char buffer[MAXRCVLEN + 1], *er; //+1 so we can add null terminator
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
	
	//checks to see if this is a large file
	if( nbyte>4000 ){
		char *re[20];
		printf("Large file will be passed\n");
		//interprets message
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
		re[part] = (char*) malloc(length + 1);
		strncpy(re[part], &buffer[length], (length + 1));
		
	}
	char *re[2];
	//interprets message
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
	er = (char*) malloc(length + 1);
	strncpy(er, &buffer[start], (length + 1));
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
	sprintf(fild,"%d",fd);
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
	netserverinit("127.0.0.1","unrestricted");
	fd = netopen("test/test.txt",O_RDWR);
	//open("test/test1.txt",O_RDWR);
	//x = netclose(fd);
	//printf("%d\n",errno);
	netread(fd, buf, 20);
	printf("%s\n",buf);
	//char *buff = "You fool! I have been trained in the Jedi arts by Count Dooku.";
	//netwrite(fd, buff, strlen(buff));
	netclose(fd);
	
	return 0;
}

