#include "TCPSocket.h"
#include "Request.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <sys/stat.h>

typedef struct HTTPManager HTTPManager;

HTTPManager * HTTPManagerCreate();
int HTTPManagerStart(HTTPManager * httpmanager, int port);
int HTTPManagerArm(HTTPManager * httpmanager, fd_set * rfds, int * maxfd );
int HTTPManagerReq(HTTPManager * httpmanager, fd_set * rfds, Request * request);