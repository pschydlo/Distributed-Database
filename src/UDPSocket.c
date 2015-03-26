#include "UDPSocket.h"

int UDPSocketCreate(){
    int fd;
    
    if((fd=socket(AF_INET,SOCK_DGRAM,0)) == -1) exit(1); /*ERROR HANDLING TO BE DONE PL0X*/
    
    return fd;
}

int UDPSocketSend(int fd, char * ip, int port, char * buffer, int n){
    struct sockaddr_in addr;
    memset(&addr,0, sizeof(struct sockaddr_in));
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr);
    
    return sendto(fd, buffer, n, 0, (struct sockaddr*)(&addr), sizeof(struct sockaddr_in));
}
