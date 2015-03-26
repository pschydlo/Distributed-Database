#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int UDPSocketCreate();
int UDPSocketSend(int fd, char * ip, int port, char * buffer, int n);