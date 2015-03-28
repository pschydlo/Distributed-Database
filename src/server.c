#include "server.h"

#define max(A,B) ((A)>=(B)?(A):(B)) /*I think we don't even use this, we just do it manually in the while loop*/

#define ID_UPPER_BOUND 64

/*--------- Start Interface Codes ---------- */

#define UI   0
#define TCP  1
#define UDP  2
#define RING 3

/*--------- Start Switching Codes ---------- */

/* UI Manager comand hashes */

#define UI_JOIN   592
#define UI_LEAVE  1253
#define UI_SHOW   657
#define UI_RSP    350
#define UI_SEARCH 2654
#define UI_BOOPP  1140
#define UI_BOOPS  1143
#define UI_STREAM 2955
#define UI_CLOSESTREAM 69707
#define UI_HASH   586
#define UI_SEND   692
#define UI_EXIT   622
#define UI_CHECK  1097
#define UI_DEBUG  1133
#define UI_STATUS 2913
#define UI_HELP   636

/* Ring Manager comand hashes */

#define RING_QRY    441
#define RING_RSP    446
#define RING_CON    476
#define RING_BOOT   998

/*TCP Manager comand hashes */

#define TCP_NEW  485
#define TCP_CON  476
#define TCP_SUCC 777
#define TCP_ID   214

/*UDP Manager comand hashes */

#define UDP_NOK 493
#define UDP_OK  213
#define UDP_EMPTY 1929
#define UDP_BRSP 942
#define JOIN_SUCC 777

/* --------- End Switching Codes ---------- */

struct Server{
    int isBoot;
    int debug;
    int shutdown;
    int TCPport;
    char ip[16];
    
    RoutingTable * routingtable;
    UDPManager  * udpmanager;
    TCPManager  * tcpmanager;
    RingManager * ringmanager;
};

Server * ServerInit(){
    Server * server=(Server*)malloc(sizeof(Server));
    
    /* -- Default configs -- */
    server->isBoot      = 0;
    server->shutdown    = 0;
    server->debug       = 1;
    
    server->udpmanager  = UDPManagerCreate();
    server->tcpmanager  = TCPManagerCreate();
    server->ringmanager = RingManagerCreate();
    server->routingtable= RoutingTableCreate(ID_UPPER_BOUND);
    
    return server;
}

int ServerStart(Server * server, char * ip, int port){
    
    /* Initial setup */
    int maxfd, counter;
    fd_set rfds;
    int n = 0;
    strcpy(server->ip, ip);
    server->TCPport  = port;
    
    Request *request = RequestCreate();
    RequestReset(request);
    
    /* Start interfaces */
    TCPManagerStart(server->tcpmanager, &(server->TCPport));
    RingManagerStart(server->ringmanager, server->ip, server->TCPport);
    UDPManagerStart(server->udpmanager);
    /*
    HTTPManager * httpmanager = HTTPManagerCreate();
    HTTPManagerStart(httpmanager, 2000);*/
    
    printf("Started server succesfully.\n");
    
    /* Event Loop */
    while(!(server->shutdown)){
        
        FD_ZERO(&rfds); maxfd=0;
        
        UIManagerArm(&rfds,&maxfd);
        RingManagerArm(server->ringmanager,&rfds,&maxfd);
        TCPManagerArm(server->tcpmanager, &rfds, &maxfd);
        UDPManagerArm(server->udpmanager, &rfds, &maxfd);
        /*HTTPManagerArm(httpmanager, &rfds, &maxfd);*/
        
        counter = select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval*)NULL);
        
        if(counter < 0){
            puts("Select error.");
            perror("select()");
            exit(1);
        }
        if(counter == 0) continue;

        do{
            n = 0;
            RequestReset(request);
            if(RingManagerReq(server->ringmanager, &rfds, request)){
                ServerProcRingReq(server, request);
                n++;
            }

            RequestReset(request);
            if(TCPManagerReq(server->tcpmanager, &rfds, request)){
                ServerProcTCPReq(server, request);
                n++;
            }

            RequestReset(request);
            if(UDPManagerReq(server->udpmanager, &rfds, request)){
                ServerProcUDPReq(server, request);
                n++;
            }
            
            RequestReset(request);
            if(UIManagerReq(&rfds, request)){
                ServerProcUIReq(server, request);
                n++;
            }
            
          /*  RequestReset(request);
            if(HTTPManagerReq(httpmanager, &rfds, request)){
                n++;
            }*/
            
        } while(n != 0);
    }
  
    RequestDestroy(request);
  
    return 0;
}

int ServerDestroy(Server * server){
    /*To do: close all active fd's*/
    
    UDPManagerStop(server->udpmanager);
    TCPManagerStop(server->tcpmanager);
    RingManagerStop(server->ringmanager, server->isBoot);
    RoutingTableDestroy(server->routingtable);
    free(server);
 
    return 0;
}

int ServerProcUDPReq(Server * server, Request * request){
    int argCount = RequestGetArgCount(request);
    if(argCount <= 0) return 0;

    
    if(server->debug){
        printf("UDP wrote: ");
        int i = 0;
        for(i = 0; i<RequestGetArgCount(request); i++){
            printf("%s ", RequestGetArg(request, i));
            fflush(stdout);
        } 
        printf("\n");
    };
    
    
    char * command = RequestGetArg(request,0);
    int code = hash(command);
    
    switch(code){
        case(UDP_OK):
        {
            if(RingManagerRing(server->ringmanager) == -1){
                RingManagerSetRing(server->ringmanager, UDPManagerRing(server->udpmanager), UDPManagerID(server->udpmanager));
                server->isBoot = 1;
            }else{
                RingManagerLeave(server->ringmanager, server->isBoot);
                server->isBoot = 0;
            }
            break;
        }
        case(UDP_NOK):
        {
            printf("Problem with UDP. Received NOK.\n");
            break;
        }
        case(UDP_EMPTY):
        {
            UDPManagerReg(server->udpmanager, server->ip, server->TCPport);
            break;
        }
        case(UDP_BRSP):
        {
            int destID      = atoi(RequestGetArg(request, 2));
            char * destIP   = RequestGetArg(request, 3);
            int destPort    = atoi(RequestGetArg(request, 4));

            int id = UDPManagerID(server->udpmanager);

            /* we create a function called UDPManagerCheckID() */
            
            if(id != destID){
                int n, tcpfd = TCPSocketCreate();
                
                if((n = TCPSocketConnect(tcpfd, destIP, destPort)) < 0){
                    printf("IP: %s, Port: %d\n", destIP, destPort);
                    puts("Could not connect to boot vertex.\n");
                    exit(1);
                } /*ERROR! checking to be done*/

                char msg[50];
                sprintf(msg, "ID %d\n", UDPManagerID(server->udpmanager));
                TCPSocketWrite(tcpfd, msg, strlen(msg));
                
                UDPManagerSetTCPfd(server->udpmanager, tcpfd);
            } else {
                printf("ID %d occupied, please choose another\n", destID);
            }
            
            break;
        }
        case(JOIN_SUCC):
        {
            int id   = UDPManagerID(server->udpmanager);
            int ring = UDPManagerRing(server->udpmanager);
            int succiID    = atoi(RequestGetArg(request, 1));
            char * succiIP = RequestGetArg(request, 2);
            int succiPort  = atoi(RequestGetArg(request, 3));
            
            printf("Got this far!\n");
            fflush(stdout);

            if(UDPManagerID(server->udpmanager) == succiID){
                printf("ID %d already in use in ring, please select another\n", succiID);
                break;
            }
            
            RingManagerSetRing(server->ringmanager, ring, id);
            RingManagerConnect(server->ringmanager, ring, id, succiID, succiIP, succiPort);
            close(UDPManagerTCPfd(server->udpmanager));
            UDPManagerSetTCPfd(server->udpmanager, -1);
            
            printf("Joining ring.\n");
            
            break;
        }
        
        default:
            break;
    }
    
    return 0;
}

/* Process Ring Manager Requests */

int ServerProcRingReq(Server * server, Request * request){
    int argCount = RequestGetArgCount(request);
    if(argCount <= 0) return 0;

    
    if(server->debug){
        printf("Ring wrote: ");
        int i = 0;
        for(i = 0; i < RequestGetArgCount(request); i++){
            printf("%s ", RequestGetArg(request, i));
            fflush(stdout);
        } 
        printf("\n");
    }
    
    
    char * command = RequestGetArg(request,0);
    int code = hash(command);
    
    switch(code){
        case(RING_RSP):
        {
            if(RequestGetArgCount(request) != 6) break; 

            int originID = atoi(RequestGetArg(request, 1));
            int searchID = atoi(RequestGetArg(request, 2));
            
            int    responsibleID    = atoi(RequestGetArg(request, 3)); 
            char * responsibleIP    = RequestGetArg(request, 4);
            int    responsiblePort  = atoi(RequestGetArg(request, 5));

            if(originID != RingManagerId(server->ringmanager)){
                RingManagerRsp(server->ringmanager, originID, searchID, responsibleID, responsibleIP, responsiblePort);
                break;
            }
                
            /*------ Handle response -----*/ 
            int interfaceID;
            
            while((interfaceID = RoutingTablePop(server->routingtable, searchID)) != -1){
                
                if(server->debug){
                    printf("RoutingTable: Handling response to interface %d\n", interfaceID); 
                    fflush(stdout);
                }

                switch(interfaceID){
                    case(UI):
                    {
                        printf("%d belongs to %d at %s %d\n", searchID, responsibleID, responsibleIP, responsiblePort);
                        fflush(stdout);
                        break;
                    }
                    case(TCP):
                    {
                        int fd = TCPManagerRoutingPop(server->tcpmanager, request, searchID);
                        char msg[50];
                        sprintf(msg, "SUCC %d %s %d\n", responsibleID, responsibleIP, responsiblePort);
                        TCPSocketWrite(fd, msg, strlen(msg));
                        break;
                    }
                }
            }
            break;
        }
        case(RING_CON):
        {
            if(RequestGetArgCount(request) != 4) return 0;
                        
            int id = RingManagerId(server->ringmanager);
            
            int    succiID   = atoi(RequestGetArg(request, 1));
            char * succiIP   = RequestGetArg(request, 2);
            int    succiPort = atoi(RequestGetArg(request, 3));
            
            puts("received a CON");
            if(succiID == id) puts("I am alone");

            RingManagerConnect(server->ringmanager, RingManagerRing(server->ringmanager), id, succiID, succiIP, succiPort);
            
            break;
        }
        case(RING_QRY):
        {
            if(RequestGetArgCount(request) != 3) break;

            int originID = atoi(RequestGetArg(request, 1));
            int searchID = atoi(RequestGetArg(request, 2));

            if(RingManagerCheck(server->ringmanager, searchID)){
                RingManagerRsp(server->ringmanager, originID, searchID, RingManagerId(server->ringmanager), server->ip, server->TCPport);
            }else{
                RingManagerQuery(server->ringmanager, originID, searchID );
            }
            break;
        }
        case(RING_BOOT):
        {
            server->isBoot = 1;
            break;
        }
        default:
            //Handle unrecognized command
            break;
    }
    
    return 0;
}

/* Process TCP Manager Requests */

int ServerProcTCPReq(Server * server, Request * request){
    int argCount = RequestGetArgCount(request);
    if(argCount <= 0) return 0;
    
    
    if(server->debug){
        printf("External wrote: ");
        int i = 0;
        for(i = 0; i<RequestGetArgCount(request); i++){
            printf("%s ", RequestGetArg(request, i));
            fflush(stdout);
        } 
        printf("\n");
        
        printf("From socket %d.\n",RequestGetFD(request));
    }
    
        
    char * command = RequestGetArg(request,0);
    int code = hash(command);
    
    switch(code){
        case(TCP_NEW):
        {
            if(RequestGetArgCount(request) != 4) break;
            
            int originID    = atoi(RequestGetArg(request, 1));
            char * originIP = RequestGetArg(request, 2);
            int originPort  = atoi(RequestGetArg(request, 3));
            
            RingManagerNew(server->ringmanager, RequestGetFD(request), originID, originIP, originPort);
            TCPManagerRemoveSocket(server->tcpmanager, RequestGetFD(request));
            
            break;
        }
        case(TCP_ID):
        {
            if(RequestGetArgCount(request) != 2) break;
            
            int searchID = atoi(RequestGetArg(request, 1));
            int nodeID   = RingManagerId(server->ringmanager);
            
            if( searchID < 0 || searchID > ID_UPPER_BOUND ) break;
                
            if(RingManagerCheck(server->ringmanager, searchID)){
                char msg[50];
                sprintf(msg, "SUCC %d %s %d\n", nodeID, server->ip, server->TCPport);
                TCPSocketWrite(RequestGetFD(request), msg, strlen(msg));

                if(server->debug){
                    printf("Trying to tell external to SUCC me off\n");
                    printf("%sto fd %d\n", msg, RequestGetFD(request));
                    fflush(stdout);
                }
            }
            else{
                RingManagerQuery(server->ringmanager, nodeID, searchID);
                UDPManagerSetTCPfd(server->udpmanager, RequestGetFD(request));
                RoutingTablePush(server->routingtable, searchID, TCP);
                TCPManagerRoutingPush(server->tcpmanager, searchID, RequestGetFD(request));
            
                if(server->debug){
                    printf("Will look for someone to SUCC external off\n");
                    fflush(stdout);
                }
            }
            break;
        }
        
        default: 
            break;
    }

    return 1;
}

/* Process UI Manager Requests */

int ServerProcUIReq(Server * server, Request * request){
    int argCount = RequestGetArgCount(request);
    if(argCount <= 0) return 0;

    
    if(server->debug){
        printf("UI wrote: ");
        int i = 0;
        for(i = 0; i<RequestGetArgCount(request); i++){
            printf("%s ", RequestGetArg(request, i));
            fflush(stdout);
        } 
        printf("\n");
        printf("From socket %d.\n",RequestGetFD(request));
    }
    
    
    char * command = RequestGetArg(request,0);
    int code = hash(command);
    
    switch(code){
        case(UI_JOIN):
        {
            if(argCount == 3){
                if(RingManagerRing(server->ringmanager) >= 0){
                    printf("Please leave the current ring before attempting a join.\n");    
                    break;
                }
                
                UDPManagerJoin(server->udpmanager, atoi(RequestGetArg(request, 1)), atoi(RequestGetArg(request, 2)));
                
                printf("Request for information sent.\n");
                fflush(stdout);
                
            }else if(argCount == 6){
            
                int ring = atoi(RequestGetArg(request, 1));
                int id   = atoi(RequestGetArg(request, 2));
                int succiID    = atoi(RequestGetArg(request, 3));
                char * succiIP = RequestGetArg(request, 4); 
                int succiPort  = atoi(RequestGetArg(request, 5)); 

                RingManagerConnect(server->ringmanager, ring, id, succiID, succiIP, succiPort);
            }
            break;
        }
        case(UI_LEAVE):
        {
            if(server->isBoot){ /*Put this in a function and place also after serverstart while loop*/
                if(RingManagerAlone(server->ringmanager)) UDPManagerRem(server->udpmanager);
                else UDPManagerRegSucc(server->udpmanager, 
                                        RingManagerSuccID(server->ringmanager), 
                                        RingManagerSuccIP(server->ringmanager), 
                                        RingManagerSuccPort(server->ringmanager));
            }else RingManagerLeave(server->ringmanager, server->isBoot);
            
            break;
        }
        case(UI_SHOW):
        {
            RingManagerStatus(server->ringmanager);
            break;
        }
        case(UI_STATUS):
        {
            printf("Ring Manager:\n");
            RingManagerStatus(server->ringmanager);
            printf("UDP Manager:\n");
            UDPManagerStatus(server->udpmanager);
            printf("Boot Status:\n");
            printf("isBoot = %d\n", server->isBoot);
            fflush(stdout);
            break;    
        }
        case(UI_RSP):
        {
            RingManagerRsp(server->ringmanager, 0, 1, RingManagerId(server->ringmanager), server->ip, server->TCPport);
            break;  
        }
        case(UI_SEARCH):
        {
            /*Reminder: limit commands if user is not connect to ring*/
            if(RequestGetArgCount(request) < 2) return 0;

            int searchID  = atoi(RequestGetArg(request, 1));
            int nodeID    = RingManagerId(server->ringmanager);

            if(searchID < 0 || searchID > ID_UPPER_BOUND) break;
            
            if( RingManagerCheck(server->ringmanager, searchID) ){
                printf("Yey, don't have to go far: %i, ip, port\n", nodeID); /*Add variables for ip and port eventually*/
            } else {
                RingManagerQuery(server->ringmanager, nodeID, searchID); /*Add int->string support eventually*/
                RoutingTablePush(server->routingtable, searchID, UI);
            }
            break;
        }
        case(UI_BOOPP):
        {
            RingManagerMsg(server->ringmanager, 1, "Boop\nBoop\n");
            break;
        }
        case(UI_BOOPS):
        {
            RingManagerMsg(server->ringmanager, 0, "Boop\nBoop\n");
            break;
        }
        case(UI_STREAM):
        {
            char buffer[100];
            fgets(buffer, 100, stdin);
            buffer[strlen(buffer)-1] = '\0';
            
            RingManagerMsg(server->ringmanager, 1, buffer);
            break;
        }
        case (UI_CLOSESTREAM):
        {
            RingManagerMsg(server->ringmanager, 1, "\n");
            break;
        }
        case(UI_HASH):
        {
            printf("hash: %i\n", hash(RequestGetArg(request, 1)));
            break;
        }
        case(UI_SEND):
        {
            char buffer[100];
            fgets(buffer, 100, stdin);
            
            RingManagerMsg(server->ringmanager, 1, buffer);
            break;
        }
        case(UI_EXIT):
        {
            if(RingManagerRing(server->ringmanager) != -1) printf("Please leave ring before exiting.\n");
            else server->shutdown = 1;
            break;
        }
        case(UI_CHECK):
        {           
            int id = atoi(RequestGetArg(request, 1));
            
            if(RingManagerCheck(server->ringmanager, id)){
                printf("It's ours");
            } else {
                printf("No luck here");
            }
            
            fflush(stdout);
            break;
        }
        case(UI_DEBUG):
        {
            if( argCount < 2 ) break;
            if( strcmp(RequestGetArg(request, 1), "off") == 0 ){
                server->debug = 0;
                
                printf("Debug mode disabled.\n");
                fflush(stdout);    
                break;
            }
            server->debug = 1;
            
            printf("Debug mode enabled.\n");
            fflush(stdout);
            break;
        }
        case(UI_HELP):
        {
            printf("There's no help for you young lad. \n");
            break;
        }
        default:
        {
            printf("Comando não reconhecido\n");
            break;
        }
    }
    
    return 0;
}

void ServerSetIP(Server * server, char* ip){
    strcpy(server->ip,ip);
}

int hash(char *str){
    int h = 0;
    while (*str)
        h = h << 1 ^ *str++;
    return h;
}

void ServerSetBootServer(Server * server, char * ip, int port){
    if (ip != NULL) UDPManagerSetIP(server->udpmanager, ip);
    if (port) UDPManagerSetPort(server->udpmanager, port);
    
    return;
}
