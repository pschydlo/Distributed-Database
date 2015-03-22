#ifndef REQUEST_H
#define REQUEST_H
#include <stdlib.h>
#include <string.h>

typedef struct Request Request;

void RequestReset(Request * request);

void RequestAddFD(Request * request, int fd);
int RequestGetFD(Request * request);

int RequestParseString(Request * request, char * buffer);
  
char * RequestGetArg(Request * request, int n);
int RequestGetArgCount(Request * request);

Request * RequestCreate();
void RequestDestroy(Request * request);

int RequestWrite(Request * request, char * buffer);

char * RequestGetBuffer(Request * request);

int RequestPushArg(Request * request, char * memstart, int length);

#endif
