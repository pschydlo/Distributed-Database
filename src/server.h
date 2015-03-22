#include "Request.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "UDPManager.h"
#include "TCPManager.h"
#include "UIManager.h"
#include "RingManager.h"


typedef struct Server Server;

Server * ServerInit(int argc, char ** argv);
int ServerProcArg(Server * server, int argc, char ** argv);
int ServerStart(Server * server);
int ServerStop(Server * server);

int ServerProcRingReq(Server * server, Request * request);
int ServerProcTCPReq(Server * server, Request * request);
int ServerProcUIReq(Server * server, Request * request);
