#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include "Buffer.h"
#include "Request.h"
#include "TCPSocket.h"
#include "RoutingTable.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


typedef struct TCPManager TCPManager;

TCPManager * TCPManagerCreate();
int TCPManagerStart(TCPManager * tcpmanager, int * TCPport);

int TCPManagerArm(TCPManager * tcpmanager, fd_set * rfds, int * maxfd);
int TCPManagerReq(TCPManager * tcpmanager, fd_set * rfds, Request * request);
void TCPManagerRemoveSocket(TCPManager * tcpmanager, int fd);
void TCPManagerStop ( TCPManager * tcpmanager);

int TCPManagerRoutingPop(TCPManager * tcpmanager, Request * request, int searchID);
void TCPManagerRoutingPush(TCPManager * tcpmanager, int searchID, int fd);
