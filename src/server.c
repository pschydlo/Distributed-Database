#include "server.h"

#define max(A,B) ((A)>=(B)?(A):(B))

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
#define RING_CONFAIL 7722

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
    server->debug       = 0;
    
    server->udpmanager  = UDPManagerCreate();
    server->tcpmanager  = TCPManagerCreate();
    server->ringmanager = RingManagerCreate();
    server->routingtable= RoutingTableCreate(ID_UPPER_BOUND);
    /*General routing table for indexing pending searches,
     * distinguishes between sources: UI or external connection*/
    
    return server;
}

int ServerStart(Server * server, char * ip, int port){
    
    /* Initial setup */
    int maxfd, counter;
    fd_set rfds;
    int n     = 0;
    int wrote = 0;
    strcpy(server->ip, ip);
    server->TCPport  = port;
    
    Request *request = RequestCreate();
    RequestReset(request);
    
    /* Start interfaces */
    TCPManagerStart(server->tcpmanager, &(server->TCPport));    /*Sets up listening port, pfd*/
    RingManagerStart(server->ringmanager, server->ip, server->TCPport);
    UDPManagerStart(server->udpmanager);                        /*Sets up UDP socket*/
    /*
    HTTPManager * httpmanager = HTTPManagerCreate();
    HTTPManagerStart(httpmanager, 2000);*/
    
    printf("###########################\nStarted server succesfully.\n");
    printf("%s:%d\n\n", server->ip, server->TCPport);
    fflush(stdout);
    
    wrote = 1;

    /* Event Loop */
    while(!(server->shutdown)){
        
        FD_ZERO(&rfds); maxfd=0;
        
        UIManagerArm(&rfds,&maxfd);
        RingManagerArm(server->ringmanager,&rfds,&maxfd);
        TCPManagerArm(server->tcpmanager, &rfds, &maxfd);
        UDPManagerArm(server->udpmanager, &rfds, &maxfd);
        /*HTTPManagerArm(httpmanager, &rfds, &maxfd);*/
        
        if(wrote == 1){
            printf("$:");
            fflush(stdout);
            wrote = 0;
        }
        
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
            if(RingManagerReq(server->ringmanager, &rfds, request) > 0){
                wrote = ServerProcRingReq(server, request);
                n++;
            }

            RequestReset(request);
            if(TCPManagerReq(server->tcpmanager, &rfds, request) > 0){
                wrote = ServerProcTCPReq(server, request);
                n++;
            }

            RequestReset(request);
            if(UDPManagerReq(server->udpmanager, &rfds, request) > 0){
                wrote = ServerProcUDPReq(server, request);
                n++;
            }
            
            RequestReset(request);
            if(UIManagerReq(&rfds, request) > 0){
                wrote = ServerProcUIReq(server, request);
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
                puts("Ring established successfully.");
            }else{
                if(RingManagerAlone(server->ringmanager)) puts("Ring successfully deleted.");
                else puts("Boot state transfered successfully.");
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
            
            fflush(stdout);

            if(UDPManagerID(server->udpmanager) == succiID){
                printf("ID %d already in use in ring, please select another\n", succiID);
                break;
            }
            
            printf("Attempting to join ring.\n");
            
            RingManagerSetRing(server->ringmanager, ring, id);
            RingManagerConnect(server->ringmanager, ring, id, succiID, succiIP, succiPort);
            close(UDPManagerTCPfd(server->udpmanager));
            UDPManagerSetTCPfd(server->udpmanager, -1);
            
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
        
        printf("$:");
        fflush(stdout);
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
                        
                        printf("$:");
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
        case(RING_CONFAIL):
        {
            RingManagerAbrupt(server->ringmanager);
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
    
    int wrote = 0;
    
    if(server->debug){
        printf("External wrote: ");
        int i = 0;
        for(i = 0; i<RequestGetArgCount(request); i++){
            printf("%s ", RequestGetArg(request, i));
            fflush(stdout);
        } 
        printf("\n");
        
        printf("From socket %d.\n",RequestGetFD(request));
        
        wrote = 1;
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
            
            if(RingManagerNew(server->ringmanager, RequestGetFD(request), originID, originIP, originPort))
                puts("Connected successfully.");
            else puts("Connection unsuccessful.");
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
                    printf("%s to fd %d\n", msg, RequestGetFD(request));
                    fflush(stdout); 
                    
                    wrote = 1;
                }                            

            }
            else{
                RingManagerQuery(server->ringmanager, nodeID, searchID);
                UDPManagerSetTCPfd(server->udpmanager, RequestGetFD(request));
                RoutingTablePush(server->routingtable, searchID, TCP);
                TCPManagerRoutingPush(server->tcpmanager, searchID, RequestGetFD(request));
            
                if(server->debug){
                    printf("This node will not be the SUCC\n");
                    fflush(stdout);
                    
                    wrote = 1;
                }
            }
            break;
        }
        
        default: 
            break;
    }
    
    return wrote;
}

/* Process UI Manager Requests */

int ServerProcUIReq(Server * server, Request * request){
    int argCount = RequestGetArgCount(request);
    if(argCount <= 0) return 0;

    int wrote = 0;
    
    if(server->debug){
        printf("UI wrote: ");
        int i = 0;
        for(i = 0; i<RequestGetArgCount(request); i++){
            printf("%s ", RequestGetArg(request, i));
            fflush(stdout);
        } 
        printf("\n");
        printf("From socket %d.\n",RequestGetFD(request));
        wrote = 1;
    }
    
    char * command = RequestGetArg(request,0);
    int code = hash(command);
    
    switch(code){
        case(UI_JOIN):
        {
            if(argCount == 3){
                if(RingManagerRing(server->ringmanager) >= 0){
                    printf("Please leave the current ring before attempting a join.\n");    
                    wrote = 1;
                    break;
                }
                
                UDPManagerJoin(server->udpmanager, atoi(RequestGetArg(request, 1)), atoi(RequestGetArg(request, 2)));
                
            }else if(argCount == 6){
            
                int ring = atoi(RequestGetArg(request, 1));
                int id   = atoi(RequestGetArg(request, 2));
                int succiID    = atoi(RequestGetArg(request, 3));
                char * succiIP = RequestGetArg(request, 4); 
                int succiPort  = atoi(RequestGetArg(request, 5)); 

                if(RingManagerConnect(server->ringmanager, ring, id, succiID, succiIP, succiPort))
                    puts("Connection successful.");
                else puts("Connection unsuccessful.");
            }
            break;
        }
        case(UI_LEAVE):
        {
            if(server->isBoot){
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
            wrote = 1;
            break;
        }
        case(UI_STATUS):
        {
            printf("Ring Manager:\n");
            RingManagerStatus(server->ringmanager);
            printf("UDP Manager:\n");
            UDPManagerStatus(server->udpmanager);
            printf("Boot State:\n");
            printf("isBoot = %d\n", server->isBoot);
            fflush(stdout);
            wrote = 1;
            break;    
        }
        case(UI_RSP):
        {
            if(RequestGetArgCount(request) != 3) break;
            int askerID  = atoi(RequestGetArg(request, 1));
            int searchID  = atoi(RequestGetArg(request, 2));
            RingManagerRsp(server->ringmanager, askerID, searchID, 
                            RingManagerId(server->ringmanager), 
                            server->ip, server->TCPport);
            break;  
        }
        case(UI_SEARCH):
        {
            if(RequestGetArgCount(request) < 2) break;

            int searchID  = atoi(RequestGetArg(request, 1));
            int nodeID    = RingManagerId(server->ringmanager);

            if(searchID < 0 || searchID > ID_UPPER_BOUND){
                printf("Requested ID is out of allowed bounds.\n");
                fflush(stdout);
                wrote = 1;
                break;
            }
            
            if(RingManagerRing(server->ringmanager) == -1){
                printf("Please join a ring before searching.\n");
                fflush(stdout);
                wrote = 1;
                break;
            }
            
            if( RingManagerCheck(server->ringmanager, searchID) ){
                printf("1 belongs to %d at %s %d\n", nodeID, server->ip, server->TCPport); 
                wrote = 1;
            } else {
                RingManagerQuery(server->ringmanager, nodeID, searchID);
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
            wrote = 1;
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
                printf("It's ours.\n");
            } else {
                printf("No luck here.\n");
            }
            wrote = 1;
            
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
                wrote = 1;
                break;
            }
            server->debug = 1;
            
            printf("Debug mode enabled.\n");
            fflush(stdout);
            
            wrote = 1;
            break;
        }
        case(UI_HELP):
        {
            puts("#     Distributed Database Project     #");
            puts("#   Computer Networking and Internet   #");
            puts("#   Instituto Superior Tecnico 2015    #");
            puts("#      Christopher Edgley  75258       #");
            puts("#      Paul Schydlo        76148       #");
            puts("");
            puts("Available commands are:");
            puts("join [ring] [id]      Joins a ring through normal procedure.");
            puts("join [ring] [id] [sucID] [sucIP] [sucPort]");
            puts("                      Joins to a node directly.");
            puts("show                  Shows ring, ID, successor's ID");
            puts("                      and predecessor's ID.");
            puts("search [id]           Searches for [id] in current ring.");
            puts("help                  Shows this help menu.");
            puts("leave                 Leaves current ring.");
            puts("exit                  Exit the program.");
            puts("debug [on|off]        Activates or deactivates debug mode,");
            puts("                      which shows network messages.");
            if(!server->debug) break;
            puts("");
            puts("Debug commands:");
            puts("status                Shows ring, ID, sucID, predID.");
            puts("                      in tcpmanager and udpmanager, boot state.");
            puts("check [id]            Checks if [id] belongs to current node.");
            puts("stream                Sends next inputted text to predi without cr termination.");
            puts("closestream           Sends only cr termination.");
            puts("boopp                 Sends two pipelined messages to predecessor.");
            puts("boops                 Sends two pipelined messages to successor.");
            puts("rsp [askerID] [searchID]");
            puts("                      Sends manual response as if [askerID] asked for [searchID].");
            puts("send                  Sends next inputted text to predecessor.");
            puts("hash [cmd]            Generates hash code for [cmd].");
            
            wrote = 1;
            break;
        }
        default:
        {
            printf("Command not recognised.\n");
            wrote = 1;
            break;
        }
    }
    
    return wrote;
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
