#include "TCPManager.h"

#define MAX_CON 100

struct TCPManager{
    int pfd;
    int sockets[MAX_CON];
    RoutingTable * localRouting;
};

TCPManager * TCPManagerCreate(){
    int i = 0;
    TCPManager * tcpmanager = (TCPManager*)malloc(sizeof(TCPManager));
    memset(tcpmanager, 0, sizeof(TCPManager));
    
    /*Creates tcp routing table, which is responsible for delivering 
     * a received RSP to the external connection which asked for it through ID*/
    tcpmanager->localRouting = RoutingTableCreate(64);
    
    /*Initialises sockets which will receive external connections asking for ID i*/
    for(i=0; i < MAX_CON; i++){
        tcpmanager->sockets[i] = -1;
    }
    
    return tcpmanager;
}

int TCPManagerStart(TCPManager * tcpmanager, int * TCPport){
    int pfd;    /*pfd is the listening socket, which will route incoming connections to tcpmanager->sockets[i]*/
    char opt;
    
    while((pfd = TCPSocketCreate()) == -1){
        printf("SocketCreate failed. Try again? [Y/n] ");
        scanf("%c", &opt);
        if(opt != 'Y' && opt != 'y') exit(1);
    }
    
    while(TCPSocketBind(pfd, *TCPport) == -1){
        printf("Port unavailable. Please choose another port: ");
        
        while(scanf("%d", TCPport) != 1){
            getchar();
            printf("Input not a number, insert a diferent port: "); 
        }
    }
    
    while(TCPSocketListen(pfd) == -1){
        printf("TCPSocketListen failed. Try again? [Y/n] ");
        scanf("%c", &opt);
        if(opt != 'Y' && opt != 'y') exit(1);
    }
    
    tcpmanager->pfd = pfd;

    return 0;
}

int TCPManagerArm( TCPManager * tcpmanager, fd_set * rfds, int * maxfd ){
    
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
        }
        
        if(i == MAX_CON){
            TCPSocketWrite(newfd, "Busy try again later.\n", 22);
            close(newfd);
        }
    }

    for(i = 0; i < MAX_CON; i++){
        if(tcpmanager->sockets[i] == -1 || !FD_ISSET(tcpmanager->sockets[i],rfds)) continue;
        FD_CLR(tcpmanager->sockets[i], rfds);
        
        if(reqcount == 1) continue;
        
        if((n = read(tcpmanager->sockets[i],buffer,128)) != 0){
            if(n <= 0){
				close(tcpmanager->sockets[i]);
				tcpmanager->sockets[i] = -1;
				continue;
			}
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

int TCPManagerRoutingPop(TCPManager * tcpmanager, Request * request, int searchID){
    return RoutingTablePop(tcpmanager->localRouting, searchID);
}

void TCPManagerRoutingPush(TCPManager * tcpmanager, int searchID, int fd){
	RoutingTablePush(tcpmanager->localRouting, searchID, fd);
}

void TCPManagerStop( TCPManager * tcpmanager ){
    int i;
    
    for(i=0; i<MAX_CON; i++){
        if(tcpmanager->sockets[i] == -1) continue;
        
        TCPSocketWrite(tcpmanager->sockets[i], "Server shutting down.\n", 23);
        close(tcpmanager->sockets[i]);
    }
    
    RoutingTableDestroy(tcpmanager->localRouting);
    
    close(tcpmanager->pfd);
    free(tcpmanager);
}
