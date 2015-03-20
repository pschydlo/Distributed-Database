#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include "Buffer.h"
#include "Request.h"

typedef struct TCPManager TCPManager;

TCPManager * TCPManagerCreate(int port);

int TCPManagerArm(TCPManager * tcpmanager, fd_set * rfds, int * maxfd);
int TCPManagerReq(TCPManager * tcpmanager, fd_set * rfds, Request * request);
