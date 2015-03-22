#include "Request.h"
#include <stdio.h>
#include <stdlib.h>
#include "TCPSocket.h"
#include <string.h>

typedef struct RingManager RingManager;

typedef struct Peer Peer;

int RingManagerId(RingManager * ringmanager);

int d(int k, int l);

int RingManagerCheck(RingManager * ringmanager, int k);

void RingManagerMsg(RingManager * ringmanager, int dest, char * msg);

int RingManagerStatus(RingManager * ringmanager);

int RingManagerConnect(RingManager * ringmanager, int ring, int id, char * ip, int port);
int RingManagerNew(RingManager * ringmanager, int fd, char * ip, int port);

int RingManagerArm(RingManager * ringmanager, fd_set * rfds, int * maxfd);

int RingManagerRes(RingManager * ringmanager, char * buffer, int nbytes);

RingManager * RingManagerInit();

int RingManagerReq(RingManager * ringmanager, fd_set * rfds, Request * request);
