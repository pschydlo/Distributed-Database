#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "Request.h"

typedef struct UDPManager UDPManager;

int UDPManagerID(UDPManager * udpmanager);

int UDPManagerRing(UDPManager * udpmanager);

int UDPManagerMsg(UDPManager * udpmanager, char * buffer);

int UDPManagerJoin(UDPManager * udpmanager, int ring, int id);

int UDPManagerSetIP(UDPManager * udpmanager, char * bootIP);

int UDPManagerSetPort(UDPManager * udpmanager, int bootPort);

int UDPManagerReg(UDPManager * udpmanager, char * ip, int port);

int UDPManagerArm( UDPManager * udpmanager, fd_set * rfds, int * maxfd );

int UDPManagerCreate(UDPManager * udpmanager);

void UDPManagerStop ( UDPManager * udpmanager);

UDPManager * UDPManagerInit();

int UDPManagerReq(UDPManager * udpmanager, fd_set * rfds, Request * request);
