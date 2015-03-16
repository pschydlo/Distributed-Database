#include <stdio.h>
#include <stdlib.h>
#include "TCPSocket.h"
#include <string.h>

typedef struct RingManager RingManager;

typedef struct Peer Peer;

int RingManagerArm(RingManager * ringmanager, fd_set * rfds, int * maxfd);

RingManager * RingManagerInit();

int RingManagerReq(RingManager * ringmanager, fd_set * rfds, char * buffer);
