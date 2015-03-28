#include "UDPSocket.h"

int UDPSocketCreate(){
    return socket(AF_INET,SOCK_DGRAM,0);
}

int UDPSocketSend(int fd, char * ip, int port, char * buffer, int n){
    struct sockaddr_in addr;
    memset(&addr,0, sizeof(struct sockaddr_in));
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &(addr.sin_addr));
    
    return sendto(fd, buffer, n, 0, (struct sockaddr*)(&addr), sizeof(struct sockaddr_in));
}
