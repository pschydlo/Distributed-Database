#include "Request.h"

struct Request{
	char buffer[128];
	char argv[10][128];
	int argc;
};

Request * RequestCreate(){
	return (Request*)malloc(sizeof(Request));
}

void RequestReset(Request * request){
	request->argc = 0;
}

int RequestPushArg(Request * request, char * memstart, int length){
	if(request->argc==10)return 0;
	
	memcpy(request->argv[request->argc], memstart, length);	
	
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
