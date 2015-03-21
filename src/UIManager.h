#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <string.h>
#include "Request.h"

int UIManagerParse(Request * request, char * buffer);

int UIManagerArm(fd_set * rfds, int * maxfd);

int UIManagerReq(fd_set *rfds, Request * request);
