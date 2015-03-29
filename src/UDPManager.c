#include "UDPManager.h"

struct UDPManager{
    int ring, id;
    char ip[16];
    int port, fd, tcpfd;
    struct sockaddr_in * addr;

};

int UDPManagerSetTCPfd(UDPManager * udpmanager, int tcpfd){
    udpmanager->tcpfd = tcpfd;
    return 0;
}

int UDPManagerTCPfd(UDPManager * udpmanager){
    return udpmanager->tcpfd;
}

int UDPManagerID(UDPManager * udpmanager){
    return udpmanager->id;
}

int UDPManagerRing(UDPManager * udpmanager){
    return udpmanager->ring;
}

int UDPManagerMsg(UDPManager * udpmanager, char * buffer){
    return UDPSocketSend(udpmanager->fd,  udpmanager->ip, udpmanager->port, buffer,  strlen(buffer));
}

int UDPManagerJoin(UDPManager * udpmanager, int ring, int id){
    char buffer[128];
    
    udpmanager->ring = ring;
    udpmanager->id   = id;
    
    sprintf(buffer, "BQRY %d", ring);
    return UDPManagerMsg(udpmanager, buffer);
}

int UDPManagerSetIP(UDPManager * udpmanager, char * bootIP){
    strcpy(udpmanager->ip, bootIP);     /*Not sure if this is correct way*/
    return 0;
}

int UDPManagerSetPort(UDPManager * udpmanager, int bootPort){
    udpmanager->port = bootPort;
    return 0;
}

int UDPManagerReg(UDPManager * udpmanager, char * ip, int port){
    char buffer[128];
    sprintf(buffer, "REG %d %d %s %d", udpmanager->ring, udpmanager->id, ip, port);
    if(UDPManagerMsg(udpmanager, buffer) == -1) exit(1); /*Error handle better, please*/
    return 0;
}

int UDPManagerRegSucc(UDPManager * udpmanager, int id, char * ip, int port){
    char buffer[128];
    sprintf(buffer, "REG %d %d %s %d", udpmanager->ring, id, ip, port);
    if(UDPManagerMsg(udpmanager, buffer) == -1) exit(1); /*Error handle better, please*/
    return 0;
}

int UDPManagerRem(UDPManager * udpmanager){
    char buffer[128];
    sprintf(buffer, "UNR %d", udpmanager->ring);
    if(UDPManagerMsg(udpmanager, buffer) == -1) exit(1); /*Error handle better, please*/
    return 0;
}

int UDPManagerArm( UDPManager * udpmanager, fd_set * rfds, int * maxfd ){
    if(udpmanager->fd > 0 ) FD_SET(udpmanager->fd, rfds);
    if(udpmanager->fd > *maxfd) *maxfd = udpmanager->fd;

    if(udpmanager->tcpfd > 0) FD_SET(udpmanager->tcpfd, rfds);
    if(udpmanager->tcpfd > *maxfd) *maxfd = udpmanager->tcpfd;
    
    return 0;
}

int UDPManagerStart(UDPManager * udpmanager){
    char opt;
    while((udpmanager->fd = UDPSocketCreate()) == -1){
        printf("UDPSocketCreate failed. Try again? [Y/n] ");
        scanf("%c", &opt);
        if(opt != 'Y' && opt != 'y') exit(1);
    }
    udpmanager->id = -1;
    udpmanager->ring = -1;
    return 0;
}

UDPManager * UDPManagerCreate(){
    UDPManager * udpmanager = (UDPManager*)malloc(sizeof(UDPManager));
    memset(udpmanager, 0, sizeof(UDPManager));
    
    /*Sets the default ip for tejo.ist.utl.pt and default access port*/
    strcpy(udpmanager->ip, "193.136.138.142");
    udpmanager->port  = 58000;
    udpmanager->tcpfd = -1;
    udpmanager->fd = -1;
    return udpmanager;
}

void UDPManagerStop ( UDPManager * udpmanager){
    close(udpmanager->fd);
    free(udpmanager);
}

int UDPManagerReq(UDPManager * udpmanager, fd_set * rfds, Request * request){

    int n = 0;
    struct sockaddr_in addr;
    
    char buffer[128];
    if(udpmanager->tcpfd != -1 && FD_ISSET(udpmanager->tcpfd,rfds)){
        FD_CLR(udpmanager->tcpfd, rfds);
        if((n = read(udpmanager->tcpfd, buffer, 128))!=0){
            if(n == -1) exit(1);
            buffer[n] = '\0';
            
            RequestParseString(request, buffer);
            RequestAddFD(request, udpmanager->tcpfd);
            return 1;
        }
    }
    
    if(udpmanager->fd != -1 && FD_ISSET(udpmanager->fd,rfds)){
        FD_CLR(udpmanager->fd, rfds);

        socklen_t addrlen = sizeof(struct sockaddr_in);

        n = recvfrom(udpmanager->fd, buffer, 128, 0, (struct sockaddr*)(&addr), &addrlen);
        if(n == -1) exit(1);

        buffer[n] = '\n';
        buffer[n+1] = '\0';

        RequestParseString(request, buffer);
        
        return 1;
    }
    
    return 0;
}

int UDPManagerStatus(UDPManager * udpmanager){
    printf("Anel %i | Id %i\n", udpmanager->ring, udpmanager->id);
    return 0;
}

void UDPManagerStartRecovery(UDPManager * udpmanager){
    
}
