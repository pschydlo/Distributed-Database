#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include "Buffer.h"

typedef struct TCPManager TCPManager;

int TCPManagerArm(TCPManager * tcpmanager, fd_set * rfds, int * maxfd);

int TCPManagerReq(TCPManager * tcpmanager, fd_set * rfds, char * buffer);
