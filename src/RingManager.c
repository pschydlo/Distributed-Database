#include "RingManager.h"

#define PREDI 0
#define SUCCI 1

struct Peer{
	int id, fd;
  char buffer[128];
	char * ip;
};

struct RingManager{
	Peer * succi;
	Peer * predi;
	int ring;
	int id;
  int TCPport;
  char ip[16];
};

int RingManagerId(RingManager * ringmanager){
	return ringmanager->id;
}

int RingManagerSetId(RingManager * ringmanager, int id){
	ringmanager->id = id;
	return 0;
}

int d(int k, int l){			/*Possibility to place this module somewhere else, for other comparisons as may be needed*/
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
	if(dest == 0 && ringmanager->succi != NULL) write(ringmanager->succi->fd, msg, strlen(msg));
	if(dest == 1 && ringmanager->predi != NULL) write(ringmanager->predi->fd, msg, strlen(msg));
}


void RingManagerQuery(RingManager * ringmanager, int askerID, int searchID ){
  char query[50];
  sprintf(query, "QRY %d %d\n", askerID, searchID);
  
  printf("Your message: %s", query);
  fflush(stdout);
  
  if(ringmanager->succi != NULL) write(ringmanager->succi->fd, query, strlen(query));
}

void RingManagerRsp(RingManager * ringmanager, int askerID, int searchID, int responsibleID, char * ip, int port){
  char query[50];
  sprintf(query, "RSP %d %d %d %s %d\n", askerID, searchID, responsibleID, ip, port);
  
  printf("Your message: %s", query);
  fflush(stdout);
  
  if(ringmanager->predi != NULL) write(ringmanager->predi->fd, query, strlen(query));
}

int RingManagerStatus(RingManager * ringmanager){
	
	printf("Anel %i | Id %i ", ringmanager->ring, ringmanager->id);
	if(ringmanager->predi != NULL) printf("| Predecessor %i ", ringmanager->predi->id);
	else printf("Nao existe predecessor ");
	if(ringmanager->succi != NULL) printf("| Successor %i", ringmanager->succi->id);
	else printf("Nao existe successor ");
	printf("| Ext facing TCP: %d", ringmanager->TCPport);
  printf("\n");
  
	
	return 0;
}

int RingManagerNew(RingManager * ringmanager, int fd, int id, char * ip, int port){
	if(ringmanager->predi == NULL){
		ringmanager->predi = (Peer*)malloc(sizeof(Peer));
	}else{
    char msg[50];
    sprintf(msg, "CON %d %s %d\n", id, ip, port); 
            
    printf("Predi: %d, ID: %d", ringmanager->predi->fd, id);
    fflush(stdout);
    
		write(ringmanager->predi->fd, msg, strlen(msg));
		close(ringmanager->predi->fd);
	}
  
  ringmanager->predi->fd = fd;
  ringmanager->predi->id = id;
  
  if(ringmanager->succi == NULL){
    RingManagerConnect(ringmanager, 1, ringmanager->id, id, ip, port);
  }
  
  return 1;
}

int RingManagerConnect(RingManager * ringmanager, int ring, int id, int succiID, char * succiIP, int succiPort){
	
	int n, fd = TCPSocketCreate();

	if((n = TCPSocketConnect(fd, succiIP, succiPort)) < 0) {
    printf("IP: %s, Port: %d", succiIP, succiPort);
		printf("Could not connect to predi.");
		exit(1);
	} /*ERROR! checking to be done*/
  
	ringmanager->id   = id;
	ringmanager->ring = ring;
	
  char msg[50];
  sprintf(msg, "NEW %d %s %d\n", id, ringmanager->ip, ringmanager->TCPport);
  
  write(fd, msg, strlen(msg));
  
	if(ringmanager->succi == NULL) ringmanager->succi = malloc(sizeof(Peer));
	ringmanager->succi->fd = fd;
  ringmanager->succi->id = succiID;
	
	return n;
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

int RingManagerRes(RingManager * ringmanager, int fd, char * buffer, int nbytes){
	
	char * ptr = buffer;
	int nwritten;
	
	while(nbytes>0){
		nwritten=write(fd,ptr,nbytes);
		if(nwritten<=0)exit(1);
		nbytes-=nwritten;
		ptr+=nwritten;
	}
  
  return 1;
}
	
RingManager * RingManagerInit(char * ip, int TCPport){
	
	RingManager * ringmanager;

	ringmanager = (RingManager*)malloc(sizeof(RingManager));
	ringmanager->succi = NULL;
	ringmanager->predi = NULL;
	ringmanager->id    = 0;		/*INITIALIZATION VALUES PLZ CHANGE*/
	ringmanager->ring  = 0;		/*TEST ONLY*/
	strcpy(ringmanager->ip, ip);
  ringmanager->TCPport = TCPport;
	
	return ringmanager;
}

int RingManagerReq(RingManager * ringmanager, fd_set * rfds, Request * request){
	int n = 0;
	
	if(ringmanager->predi!=NULL && FD_ISSET(ringmanager->predi->fd,rfds)){
		if((n=read(ringmanager->predi->fd,ringmanager->predi->buffer,128))!=0){
			if(n==-1) exit(1);				/*ERROR HANDLING PLZ DO SMTHG EVENTUALLY*/
			ringmanager->predi->buffer[n] = '\0';
      
			RequestParseString(request, ringmanager->predi->buffer);
			
			return 1;
		}
	}
	
	if(ringmanager->succi!=NULL && FD_ISSET(ringmanager->succi->fd,rfds)){
		if((n=read(ringmanager->succi->fd,ringmanager->succi->buffer,128))!=0){
			if(n==-1)exit(1);				/*ERROR HANDLING PLZ DO SMTHG EVENTUALLY*/
			ringmanager->succi->buffer[n] = '\0';
      
      /* Check if request is completely in buffer! (could happen that he only receives half \n */
			
      RequestParseString(request, ringmanager->succi->buffer);
			
      return 1;
		}
	}
	
	return 0;
}
