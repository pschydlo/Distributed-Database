#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "Request.h"

typedef struct UDPManager UDPManager;

int UDPManagerSetIP(UDPManager * udpmanager, char * bootIP);

int UDPManagerSetPort(UDPManager * udpmanager, int bootPort);

int UDPManagerJoin(UDPManager * udpmanager, int ring);

int UDPManagerCreate(UDPManager * udpmanager);

void UDPManagerStop ( UDPManager * udpmanager);

UDPManager * UDPManagerInit();
