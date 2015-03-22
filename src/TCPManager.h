#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include "Buffer.h"
#include "Request.h"
#include "TCPSocket.h"
#include <unistd.h>

typedef struct TCPManager TCPManager;

TCPManager * TCPManagerInit();
int TCPManagerStart(TCPManager * tcpmanager, int TCPport);

int TCPManagerArm(TCPManager * tcpmanager, fd_set * rfds, int * maxfd);
int TCPManagerReq(TCPManager * tcpmanager, fd_set * rfds, Request * request);
void TCPManagerRemoveSocket(TCPManager * tcpmanager, int fd);