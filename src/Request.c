#include "Request.h"

struct Request{
	char buffer[128];
	char argv[10][128];
	int argc;
};

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
