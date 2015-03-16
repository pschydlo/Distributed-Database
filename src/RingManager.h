#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

typedef struct RingManager RingManager;

typedef struct Peer Peer;

int RingManagerArm(RingManager * ringmanager, fd_set * rfds, int * maxfd);

RingManager * RingManagerInit();

void RingManagerReq(RingManager * ringmanager, fd_set * rfds, char * buffer);
