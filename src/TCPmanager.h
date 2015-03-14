#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

typedef struct TCPmanager TCPmanager;

typedef struct Peer Peer;

TCPmanager * TCPmanagerInit();

void TCPmanagerReq(TCPmanager * tcpmanager, char * buffer);
