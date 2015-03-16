#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

int UImanagerArm(fd_set * rfds, int * maxfd);

void UImanagerReq(fd_set * rfds, char * buffer);
