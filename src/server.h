#include <stdlib.h>
#include <stdio.h>
#include "RingManager.h"
#include "UImanager.h"

typedef struct Server Server;

Server * ServerInit();
int ServerStart(Server *);
int ServerStop(Server * server);

int ServerProcRingReq(Server * server, char * buffer, int n);
int ServerProcUIReq(Server * server, char * buffer, int n);
