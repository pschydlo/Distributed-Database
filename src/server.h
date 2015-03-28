#include "UDPManager.h"
#include "TCPManager.h"
#include "HTTPManager.h"
#include "UIManager.h"
#include "RingManager.h"
#include "RoutingTable.h"
#include "Request.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

typedef struct Server Server;

Server * ServerInit();
int ServerProcArg(Server * server, int argc, char ** argv);
int ServerStart(Server * server, char * ip, int port);
int ServerDestroy(Server * server);

void ServerSetBootServer(Server * server, char * ip, int port);

int ServerProcUDPReq(Server * server, Request * request);
int ServerProcRingReq(Server * server, Request * request);
int ServerProcTCPReq(Server * server, Request * request);
int ServerProcUIReq(Server * server, Request * request);

int hash(char * input);