#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stddef.h>
#include <math.h>
//#include <fcntl.h>
#define MAXRCVLEN 4500
#define PORTNUM 2343

#define O_WRONLY 1
#define O_RDONLY 0
#define O_RDWR 2


int netserverinit(const char* hostname, const char* mode);
int netopen(const char *pathname, int flags);
size_t netread(int fildes, void *buf, size_t nbyte);
int netclose(int fd);