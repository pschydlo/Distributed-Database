#include <stdlib.h>
#include <stdio.h>
#include "TCPmanager.h"
#include "UImanager.h"

typedef struct Server Server;

Server * ServerInit();
int ServerStart(Server *);
int ServerStop(Server * server);
