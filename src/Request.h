#ifndef REQUEST_H
#define REQUEST_H
#include <stdlib.h>
#include <string.h>

typedef struct Request Request;

char * RequestGetArg(Request * request, int n);
int RequestGetArgCount(Request * request);

Request * RequestCreate();

int RequestWrite(Request * request, char * buffer);

char * RequestGetBuffer(Request * request);

#endif
