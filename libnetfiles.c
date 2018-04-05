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


netserverinit(const char* hostname, const char* mode){
	
	return 0;
}

int netopen(const char *pathname, int flags){
}

size_t netread(int fildes, void *buf, size_t nbyte){
}

size_t netread(int fildes, void *buf, size_t nbyte){
}

int netclose(int fd){
}