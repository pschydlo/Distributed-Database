#include "Request.h"

struct Request{
    char buffer[128];
    char argv[10][128];
    int argc;
    int fd;
};

int RequestParseString(Request * request, char * buffer){
    int n, start = 0;
    RequestReset(request);
    
    for(n = 0; buffer[n] != '\0'; n++){
        if(buffer[n] != ' ' && buffer[n] != '\n') continue;
    
        RequestPushArg(request, buffer + start, n - start);
        start = n + 1;
     
        if(buffer[n] == '\n') return n+1;
    }
    
    return 0;
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

    /*A request serves the purpose of storing the information in any command,
     * placing each argument in an array of strings.
     * Then the arguments can be read individually.*/
Request * RequestCreate(){
    Request * request = (Request*)malloc(sizeof(Request));
    memset(request, 0, sizeof(Request));
    
    return request;
}

void RequestDestroy(Request * request){
    free(request);
}

void RequestReset(Request * request){
    memset(request, 0, sizeof(Request));
    
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
