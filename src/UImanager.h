#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <string.h>
#include "Request.h"

int UImanagerParse(Request * request, char * buffer);

int UImanagerArm(fd_set * rfds, int * maxfd);

int UImanagerReq(fd_set *rfds, Request * request);
