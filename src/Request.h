#ifndef REQUEST_H
#define REQUEST_H
#include <stdlib.h>
#include <string.h>

typedef struct Request Request;

Request * RequestCreate();

int RequestWrite(Request * request, char * buffer);

char * RequestGetBuffer(Request * request);

#endif
