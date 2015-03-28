#include "TCPSocket.h"

int TCPSocketWrite(int fd, char * buffer, int nbytes){
    char * ptr = buffer;
    int nwritten;

    while(nbytes>0){
        nwritten=write(fd,ptr,nbytes);
        if(nwritten<=0) return -1;
        nbytes-=nwritten;
        ptr+=nwritten;
    }
    
    return 1;
}

int TCPSocketAccept(int fd){
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    
    return accept(fd,(struct sockaddr*)&addr,&addrlen);
}

int TCPSocketListen(int fd){
    return listen(fd, 5);
}

int TCPSocketBind(int fd, int port){
    
    struct sockaddr_in addr;

    memset((void*)&addr,(int)'\0',sizeof(addr));
    
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htons(INADDR_ANY);
    addr.sin_port        = htons(port);
    
    return bind(fd,(struct sockaddr*)&addr,sizeof(addr));
}

int TCPSocketCreate(){
    return socket(AF_INET,SOCK_STREAM,0);
}
    
int TCPSocketConnect(int fd, char * ip, int port){
    
    struct sockaddr_in addr;

    memset((void*)&addr,(int)'\0',sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    inet_pton(AF_INET, ip, &(addr.sin_addr));

    return connect(fd,(struct sockaddr*)&addr,sizeof(addr));
}




