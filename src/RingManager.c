#include "RingManager.h"

#define PREDI 0
#define SUCCI 1

struct Peer{
    int id, fd;
    char buffer[128];
    int bufferhead;
    char ip[16];
    int port;
};

struct RingManager{
    Peer * succi;
    Peer * predi;
    int ring;
    int id;
    int TCPport;
    char ip[16];
    int watchlist;
};

int RingManagerSuccID(RingManager * ringmanager){
    if(ringmanager->succi != NULL) return ringmanager->succi->id;
    return -1;
}

char * RingManagerSuccIP(RingManager * ringmanager){
    if(ringmanager->succi != NULL) return ringmanager->succi->ip;
    return "-1";
}

int RingManagerSuccPort(RingManager * ringmanager){
    if(ringmanager->succi != NULL) return ringmanager->succi->port;
    return -1;
}

int RingManagerAlone(RingManager * ringmanager){
    return (ringmanager->predi == NULL && ringmanager->succi == NULL);
}

int RingManagerId(RingManager * ringmanager){
    return ringmanager->id;
}

int RingManagerRing(RingManager * ringmanager){
    return ringmanager->ring;
}

int RingManagerSetRing(RingManager * ringmanager, int ring, int id){
    ringmanager->ring = ring;
    ringmanager->id = id;
    return 0;
}

int d(int k, int l){
    if((l-k) < 0) return (64+l-k);
    return (l-k);
}

int RingManagerCheck(RingManager * ringmanager, int k){
    int id = ringmanager->id;
    if (ringmanager->predi == NULL) return -1;

    int predid = ringmanager->predi->id;
          
    if(d(k, id) < d(k, predid)) return 1;
    return 0;
}

void RingManagerMsg(RingManager * ringmanager, int dest, char * msg){
    if(dest == 0 && ringmanager->succi != NULL) TCPSocketWrite(ringmanager->succi->fd, msg, strlen(msg));
    if(dest == 1 && ringmanager->predi != NULL) TCPSocketWrite(ringmanager->predi->fd, msg, strlen(msg));
}


void RingManagerQuery(RingManager * ringmanager, int askerID, int searchID ){
    char query[50];
    sprintf(query, "QRY %d %d\n", askerID, searchID);
  
    if(ringmanager->succi != NULL) TCPSocketWrite(ringmanager->succi->fd, query, strlen(query));
}

void RingManagerRsp(RingManager * ringmanager, int askerID, int searchID, int responsibleID, char * ip, int port){
    char query[50];
    sprintf(query, "RSP %d %d %d %s %d\n", askerID, searchID, responsibleID, ip, port);

    if(ringmanager->predi != NULL) TCPSocketWrite(ringmanager->predi->fd, query, strlen(query));
}

int RingManagerStatus(RingManager * ringmanager){
        
    printf("Ring ");
    if(ringmanager->ring == -1) printf("-- ");
    else printf("%i ", ringmanager->ring);
    printf("| Id ");
    if(ringmanager->id == -1) printf("-- ");
    else printf("%i ", ringmanager->id);
    if(ringmanager->predi != NULL) printf("| Predecessor %i ", ringmanager->predi->id);
    else printf("| --- ");
    if(ringmanager->succi != NULL) printf("| Successor %i ", ringmanager->succi->id);
    else printf("| --- ");
    printf("| TCP port: %d", ringmanager->TCPport);
    printf("\n");
        
    return 0;
}

int RingManagerNew(RingManager * ringmanager, int fd, int id, char * ip, int port){
    struct linger so_linger;

    so_linger.l_onoff = 1;
    so_linger.l_linger = 10;
    setsockopt(fd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));

    if(ringmanager->predi == NULL){
        ringmanager->predi = (Peer*)malloc(sizeof(Peer));
        memset(ringmanager->predi, 0, sizeof(Peer));
                
        ringmanager->predi->buffer[0]  = '\0';
        ringmanager->predi->bufferhead = 0;
    }else{
        char msg[50];
        sprintf(msg, "CON %d %s %d\n", id, ip, port); 

        TCPSocketWrite(ringmanager->predi->fd, msg, strlen(msg));
        close(ringmanager->predi->fd);
    }
  
    ringmanager->predi->fd = fd;
    ringmanager->predi->id = id;
    ringmanager->predi->port = port;
    
    if(ringmanager->succi == NULL){
        return RingManagerConnect(ringmanager, ringmanager->ring, ringmanager->id, id, ip, port);
    }

    return 1;
}

int RingManagerConnect(RingManager * ringmanager, int ring, int id, int succiID, char * succiIP, int succiPort){

    if(succiID == id){
        puts("Node left.");
        /* Last node in ring */
        close(ringmanager->predi->fd);
        close(ringmanager->succi->fd);
        
        free(ringmanager->predi);
        free(ringmanager->succi);
        
        ringmanager->predi = NULL;
        ringmanager->succi = NULL;
        
        return 0;
    }
    
    int n, fd = TCPSocketCreate();
    
    if((n = TCPSocketConnect(fd, succiIP, succiPort)) < 0) {
        printf("Could not connect to succi. Please try again later.\n");
        fflush(stdout);
        return 0;
    }

    ringmanager->id   = id;
    ringmanager->ring = ring;
    
    char msg[50];
    sprintf(msg, "NEW %d %s %d\n", id, ringmanager->ip, ringmanager->TCPport);
    TCPSocketWrite(fd, msg, strlen(msg));

    if(ringmanager->succi == NULL){
        ringmanager->succi = malloc(sizeof(Peer));
    }else{
        puts("Topology change.");
        close(ringmanager->succi->fd);
    }
    
    memset(ringmanager->succi, 0, sizeof(Peer));
    
    ringmanager->succi->fd = fd;
    ringmanager->succi->id = succiID;
    ringmanager->succi->bufferhead = 0;
    ringmanager->succi->buffer[0] = '\0';
    strcpy(ringmanager->succi->ip, succiIP);
    ringmanager->succi->port = succiPort;
    
    return 1;
}

int RingManagerArm( RingManager * ringmanager, fd_set * rfds, int * maxfd ){
    
    int n = 0;
    
    if(ringmanager->predi != NULL){
        FD_SET(ringmanager->predi->fd, rfds);
        *maxfd = ( ringmanager->predi->fd > *maxfd ? ringmanager->predi->fd : *maxfd );
        n++;
    }
    
    if(ringmanager->succi != NULL){
        FD_SET(ringmanager->succi->fd, rfds);
        *maxfd = ( ringmanager->succi->fd > *maxfd ? ringmanager->succi->fd : *maxfd );
        n++;
    }
    
    return n;
}

int RingManagerStart(RingManager * ringmanager, char * ip, int TCPport){
    strcpy(ringmanager->ip, ip);
    ringmanager->TCPport = TCPport;
    return 0;
}

RingManager * RingManagerCreate(){

    RingManager * ringmanager = (RingManager*)malloc(sizeof(RingManager));
    memset(ringmanager, 0, sizeof(RingManager));
    
    /*Ring manager initialisation*/
    ringmanager->succi = NULL;
    ringmanager->predi = NULL;
    ringmanager->id    = -1;
    ringmanager->ring  = -1;
    
    return ringmanager;
}

int RingManagerReq(RingManager * ringmanager, fd_set * rfds, Request * request){
    int n = 0;
    int reqlength = 0;
    
    /* ------ Process buffers ------------ */
    if(ringmanager->succi != NULL && (reqlength = RequestParseString(request, ringmanager->succi->buffer)) != 0 ){
        memcpy(ringmanager->succi->buffer, ringmanager->succi->buffer + reqlength, ringmanager->succi->bufferhead - reqlength);
        
        ringmanager->succi->bufferhead = ringmanager->succi->bufferhead - reqlength;
        ringmanager->succi->buffer[ringmanager->succi->bufferhead] = '\0';
        return 1;
    } else RequestReset(request);
    
    if(ringmanager->predi != NULL && (reqlength = RequestParseString(request, ringmanager->predi->buffer)) != 0 ){
        memcpy(ringmanager->predi->buffer, ringmanager->predi->buffer + reqlength, ringmanager->predi->bufferhead - reqlength);

        ringmanager->predi->bufferhead = ringmanager->predi->bufferhead - reqlength;
        ringmanager->predi->buffer[ringmanager->predi->bufferhead] = '\0';
        return 1;
    } else RequestReset(request);
    
    /* ----- Fill buffers ----------------*/
    if(ringmanager->predi!=NULL && FD_ISSET(ringmanager->predi->fd,rfds)){
        FD_CLR(ringmanager->predi->fd, rfds);
        
        n = read(ringmanager->predi->fd, ringmanager->predi->buffer + ringmanager->predi->bufferhead, 128);
        
        if( n <= 0 ) {
            close(ringmanager->predi->fd);
            free(ringmanager->predi);
            ringmanager->predi = NULL;

            printf("Lost connection to predi.\n");
            fflush(stdout);
            
            RequestReset(request);
            RequestPushArg(request, "CONFAIL", 7);
            RequestPushArg(request, "predi", 5);
            return 1;
        }else{
            ringmanager->predi->bufferhead += n;
            ringmanager->predi->buffer[ringmanager->predi->bufferhead] = '\0';
            return 1;
        }
    }
    
    if(ringmanager->succi!=NULL && FD_ISSET(ringmanager->succi->fd,rfds)){
        FD_CLR(ringmanager->succi->fd, rfds);
        
        n = read(ringmanager->succi->fd, ringmanager->succi->buffer + ringmanager->succi->bufferhead, 128);
        if( n <= 0 ) {
            close(ringmanager->succi->fd);
            free(ringmanager->succi);
            ringmanager->succi = NULL;

            printf("Lost connection to succi.\n");
            fflush(stdout);
        
            RequestReset(request);
            RequestPushArg(request, "CONFAIL", 7);
            RequestPushArg(request, "succi", 5);
            return 1;
        }else{
            ringmanager->succi->bufferhead += n;
            ringmanager->succi->buffer[ringmanager->succi->bufferhead] = '\0';
            return 1;
        }
    }
    
    return 0;
}

void RingManagerLeave(RingManager * ringmanager, int isBoot){
    char msg[50];
    
    if(ringmanager->ring == -1) printf("You are not in a ring. No need to leave.\n");
    ringmanager->id = -1;
    ringmanager->ring = -1;
    
    if(ringmanager->predi == NULL && ringmanager->succi == NULL) return;
    if(ringmanager->predi == NULL || ringmanager->succi == NULL){
        printf("Ring is broken.\n");/*Something broke in this case*/
        exit(1);
    }
    
    if(isBoot){
        sprintf(msg,"BOOT\n");
        TCPSocketWrite(ringmanager->succi->fd, msg, strlen(msg));
    }
    
    sprintf(msg, "CON %d %s %d\n", ringmanager->succi->id, ringmanager->succi->ip, ringmanager->succi->port);
    TCPSocketWrite(ringmanager->predi->fd, msg, strlen(msg));
    /*sleep(1);*/
    /*shutdown(ringmanager->predi->fd, SHUT_WR);*/
    /*ringmanager->watchlist = ringmanager->predi->fd;*/
    close(ringmanager->predi->fd);
    puts("Closing predi");
    
    /*shutdown(ringmanager->succi->fd, SHUT_WR);*/
    close(ringmanager->succi->fd);
    puts("Closing succi");
    
    free(ringmanager->predi);
    free(ringmanager->succi);
    
    ringmanager->predi = NULL;
    ringmanager->succi = NULL;
    
    printf("Sucessfully left ring.\n");
    fflush(stdout);
};

void RingManagerStop ( RingManager * ringmanager, int isBoot){
    if(RingManagerRing(ringmanager) != -1) RingManagerLeave(ringmanager, isBoot);
    
    if(ringmanager->predi != NULL) free(ringmanager->predi);
    if(ringmanager->succi != NULL) free(ringmanager->succi);
    
    free(ringmanager);
}

void RingManagerAbrupt(RingManager * ringmanager){
    if(ringmanager->predi != NULL) close(ringmanager->predi->fd);
    if(ringmanager->succi != NULL) close(ringmanager->succi->fd);
}
