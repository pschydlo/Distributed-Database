#include "Request.h"

struct Request{
	char buffer[128];
	char argv[10][128];
	int argc;
  int fd;
};

int RequestParseString(Request * request, char * buffer){
	int n, start = 0;
	
	for(n = 0; buffer[n] != '\0'; n++){
		if(buffer[n] != ' ' && buffer[n]!= '\n') continue;
    
		RequestPushArg(request, buffer + start, n - start);
		start = n + 1;
	 
    if(buffer[n] == '\n') return 1;
  }
	
	return -1;
}

void RequestAddFD(Request * request, int fd){
  request->fd = fd;
}
  
int RequestGetFD(Request * request){
  return request->fd;
}

char * RequestGetArg(Request * request, int n){
	return request->argv[n];
}

int RequestGetArgCount(Request * request){
	return request->argc;
}

Request * RequestCreate(){
	return (Request*)malloc(sizeof(Request));
}

void RequestReset(Request * request){
	request->argc = 0;
  request->fd   = -1;
}

int RequestPushArg(Request * request, char * memstart, int length){
	if(request->argc>=10 || length == 0)return 0;
	
	memcpy(request->argv[request->argc], memstart, length);
	request->argv[request->argc][length] = '\0';
	request->argc++;
	
	return 1;
}

int RequestWrite(Request * request, char * buffer){
	strcpy(request->argv[0],buffer);
	request->argc=1;
	return 1;
}

char * RequestGetBuffer(Request * request){
	return request->buffer;
}
