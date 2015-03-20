#include <stdlib.h>
#include <stdio.h>
#include "RingManager.h"
#include "UImanager.h"
#include "TCPManager.h"
#include "Request.h"

typedef struct Server Server;

Server * ServerInit();
int ServerStart(Server *);
int ServerStop(Server * server);

int ServerProcRingReq(Server * server, char * buffer, int n);
int ServerProcTCPReq(Server * server, Request * request);
int ServerProcUIReq(Server * server, Request * request);
