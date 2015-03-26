#include "UDPManager.h"

struct UDPManager{
    int ring, id;
    char ip[16];
    int port, fd, tcpfd;
    struct sockaddr_in * addr;
    
};

int UDPManagerID(UDPManager * udpmanager){
    return udpmanager->id;
}

int UDPManagerRing(UDPManager * udpmanager){
    return udpmanager->ring;
}

int UDPManagerMsg(UDPManager * udpmanager, char * buffer){
    return UDPSocketSend(udpmanager->fd, buffer, strlen(buffer), udpmanager->ip, udpmanager->port);
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
    printf("%s\n", buffer);
    fflush(stdout);
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
    FD_SET(udpmanager->fd, rfds);
    if(udpmanager->fd > *maxfd) *maxfd = udpmanager->fd;
    return 0;
}

int UDPManagerStart(UDPManager * udpmanager){
    
    udpmanager->fd = UDPSocketCreate();
    return 0;
}

UDPManager * UDPManagerInit(){
    UDPManager * udpmanager = (UDPManager*)malloc(sizeof(UDPManager));
    memset(udpmanager, 0, sizeof(UDPManager));
    
    strcpy(udpmanager->ip, "193.136.138.142");  /*ip de tejo.tecnico.ulisboa.pt*/
    udpmanager->port = 58000;
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
    
    if(!FD_ISSET(udpmanager->fd, rfds)) return 0;
    FD_CLR(udpmanager->fd, rfds);
    
    socklen_t addrlen = sizeof(struct sockaddr_in);

    n = recvfrom(udpmanager->fd, buffer, 128, 0, (struct sockaddr*)&addr, &addrlen);
    if(n == -1) exit(1);
    
    buffer[n] = '\n';
    buffer[n+1] = '\0';
    
    RequestParseString(request, buffer);
    
    return 1;
}

int UDPManagerStatus(UDPManager * udpmanager){
    printf("Anel %i | Id %i\n", udpmanager->ring, udpmanager->id);
    return 0;
}
