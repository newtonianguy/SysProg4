#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stddef.h>

int netserverinit(const char* hostname, const char* mode);
int netopen(const char *pathname, int flags);
size_t netread(int fildes, void *buf, size_t nbyte);
int netclose(int fd);