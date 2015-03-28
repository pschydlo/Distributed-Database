#include "TCPManager.h"

#define MAX_CON 2

struct TCPManager{
    int pfd, idfd;
    int searchid;
    int sockets[MAX_CON];
    RoutingTable * localRouting;
    /*Buffer *buffer;*/
};

int TCPManagerIDfd(TCPManager * tcpmanager){
    return tcpmanager->idfd;
}

int TCPManagerSearchID(TCPManager * tcpmanager){
    return tcpmanager->searchid;
}

int TCPManagerSetSearch(TCPManager * tcpmanager, int idfd, int searchid){ /*We can probably fit this in another module I can't quite visualise now*/
    tcpmanager->idfd      = idfd;
    tcpmanager->searchid  = searchid;
    return 0;
}

TCPManager * TCPManagerInit(){
    int i = 0;
    TCPManager * tcpmanager = (TCPManager*)malloc(sizeof(TCPManager));
    memset(tcpmanager, 0, sizeof(TCPManager));
    
    tcpmanager->localRouting = RoutingTableCreate(64);
    
    for(i=0; i < MAX_CON; i++){
        tcpmanager->sockets[i] = -1;
    }
    tcpmanager->idfd = -1;
    
    return tcpmanager;
}

int TCPManagerStart(TCPManager * tcpmanager, int * TCPport){
    int pfd;
    char opt;
    
    while((pfd = TCPSocketCreate()) == -1){
        scanf("SocketCreate failed. Try again? [Y/n] %c", &opt);
        if(opt != 'Y' && opt != 'y') exit(1);
    }
    
    while(TCPSocketBind(pfd, *TCPport) == -1){
        printf("Choose another port: ");
        scanf("Text %d", TCPport);
    }
    
    while(TCPSocketListen(pfd) == -1){
        scanf("TCPSocketListen failed. Try again? [Y/n] %c", &opt);
        if(opt != 'Y' && opt != 'y') exit(1);
    }
    
    tcpmanager->pfd = pfd;

    return 0;
}

int TCPManagerArm( TCPManager * tcpmanager, fd_set * rfds, int * maxfd ){

    if(tcpmanager->idfd != -1) FD_SET(tcpmanager->idfd, rfds);      /*temp id socket for outgoing ID asking*/
    if(tcpmanager->idfd > *maxfd) *maxfd = tcpmanager->idfd;
    
    FD_SET(tcpmanager->pfd, rfds);
    if(tcpmanager->pfd > *maxfd) *maxfd = tcpmanager->pfd;
    
    int i = 0;
    
    for(i = 0; i < MAX_CON; i++){
        if(tcpmanager->sockets[i] == -1) continue;
    
        FD_SET(tcpmanager->sockets[i], rfds);
        if(tcpmanager->sockets[i] > *maxfd) *maxfd = tcpmanager->sockets[i];
    }
    
    return 0;
}

int TCPManagerReq(TCPManager * tcpmanager, fd_set * rfds, Request * request){

    int i = 0, n = 0;
    int reqcount = 0;
    
    char buffer[128];
    
    if(FD_ISSET(tcpmanager->pfd,rfds)){
        FD_CLR(tcpmanager->pfd, rfds);
        
        int newfd = TCPSocketAccept(tcpmanager->pfd);
        
        for(i = 0; i < MAX_CON; i++){
            if(tcpmanager->sockets[i] != -1) continue;
            
            tcpmanager->sockets[i] = newfd;
            break;
            /*if(tcpmanager->sockets[i] ==-1){
                 * tpcmanager->sockets[i] = newfd;
                 * break;
            * }*/
        }
        
        if(i == MAX_CON){
            write(newfd, "Busy try again later.\n", 22);
            close(newfd);
        }
    }
    
        
    if(tcpmanager->idfd != -1 && FD_ISSET(tcpmanager->idfd,rfds)){
        FD_CLR(tcpmanager->idfd, rfds);
        if((n=read(tcpmanager->idfd,buffer,128))!=0){
            if(n==-1)exit(1);
            buffer[n]='\0';
            
            RequestParseString(request, buffer);
            RequestAddFD(request, tcpmanager->idfd);
            
            reqcount = 1;
        }
    }
    

    for(i = 0; i < MAX_CON; i++){
        if(tcpmanager->sockets[i] == -1 || !FD_ISSET(tcpmanager->sockets[i],rfds)) continue;
        FD_CLR(tcpmanager->sockets[i], rfds);
        
        n = send(tcpmanager->sockets[i], " ", 1, MSG_NOSIGNAL);
        if (n == -1)
        {
            close(tcpmanager->sockets[i]);
            tcpmanager->sockets[i] = -1;
        
            continue;
        }
        
        if(reqcount == 1) continue;
        
        if((n=read(tcpmanager->sockets[i],buffer,128))!=0){
            if(n==-1)exit(1);               /*ERROR HANDLING PLZ DO SMTHG EVENTUALLY*/
            buffer[n]='\0';
            
            RequestParseString(request, buffer);
            RequestAddFD(request, tcpmanager->sockets[i]);
                
            reqcount = 1;
        }
    }

    return reqcount;
}

void TCPManagerRemoveSocket(TCPManager * tcpmanager, int fd){
    int i;

    for(i = 0; i<MAX_CON; i++){
        if(tcpmanager->sockets[i] != fd) continue;  
        
        tcpmanager->sockets[i] = -1;
        break;
    }
}

int TCPManagerRespond(Request * request, int fd){

    
    
    return 1;
}

void TCPManagerStop ( TCPManager * tcpmanager){
    int i;
    
    for(i=0; i<MAX_CON; i++){
        if(tcpmanager->sockets[i] == -1) continue;
        
        write(tcpmanager->sockets[i], "Server shutting down.\n", 23);
        close(tcpmanager->sockets[i]);
    }
    
    close(tcpmanager->pfd);
    free(tcpmanager);
}
