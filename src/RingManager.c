#include "RingManager.h"

#define PREDI 0
#define SUCCI 1

struct Peer{
	int id, fd;
	char * ip;
};

struct RingManager{
	Peer * succi;
	Peer * predi;
	int ring;
	int id;
};

int RingManagerId(RingManager * ringmanager){	/*Necessary function - will probably remain*/
	return ringmanager->id;
}

int d(int k, int l){			/*Possibility to place this module somewhere else, for other comparisons as may be needed*/
	return (l-k);
	if((l-k) < 0) return (64+l-k);
}

int RingManagerCheck(RingManager * ringmanager, int k){	/*Quick un-modularized function, can be substituted by more general function*/
	int id = ringmanager->id;
  int predid = (ringmanager->predi ? ringmanager->predi->id : 0);  /*Check if predi exists or not!!*/
	
	if(d(k, id) < d(k, predid)) return 1;
	return 0;
}

void RingManagerMsg(RingManager * ringmanager, int dest, char * msg){
	if(dest == 0 && ringmanager->succi != NULL) write(ringmanager->succi->fd, msg, strlen(msg));
	if(dest == 1 && ringmanager->predi != NULL) write(ringmanager->predi->fd, msg, strlen(msg));
}


void RingManagerQuery(RingManager * ringmanager, int id){
  char query[20];
  sprintf(query, "QRY %d %d\n", id, id);
  
  printf("Your query: %s", query);
  fflush(stdout);
  
  if(ringmanager->succi != NULL) write(ringmanager->succi->fd, query, strlen(query));
}

int RingManagerStatus(RingManager * ringmanager){
	
	printf("Anel %i | Id %i ", ringmanager->ring, ringmanager->id);
	if(ringmanager->predi != NULL) printf("| Predecessor %i ", ringmanager->predi->id);
	else printf("Nao existe predecessor ");
	if(ringmanager->succi != NULL) printf("| Successor %i", ringmanager->succi->id);
	else printf("Nao existe successor ");
	printf("\n");
	
	return 0;
}

int RingManagerNew(RingManager * ringmanager, int fd, char * ip, int port){
	if(ringmanager->predi == NULL){
		ringmanager->predi = (Peer*)malloc(sizeof(Peer));
	}else{
		write(ringmanager->predi->fd, "CON i i.IP i.TCP\n", 17);
		close(ringmanager->predi->fd);
	}
  
  ringmanager->predi->fd = fd;
}

int RingManagerConnect(RingManager * ringmanager, int ring, int id, char * ip, int port){
	
	int n, fd = TCPSocketCreate();

	if((n = TCPSocketConnect(fd, ip, port)) < 0) {
		printf("Could not connect to predi.");
		exit(1);
	} /*ERROR! checking to be done*/
	ringmanager->id   = id;
	ringmanager->ring = ring;
	
  write(fd, "NEW i i.IP i.PORT\n", 17);
  
	if(ringmanager->succi == NULL) ringmanager->succi = malloc(sizeof(Peer));
	ringmanager->succi->fd = fd;
	
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

int RingManagerRes(RingManager * ringmanager, char * buffer, int nbytes){
	
	char * ptr = buffer;
	int nwritten;
	int fd;
	
	while(nbytes>0){
		nwritten=write(fd,ptr,nbytes);
		if(nwritten<=0)exit(1);
		nbytes-=nwritten;
		ptr+=nwritten;
	}
}
	

RingManager * RingManagerInit(){
	
	int addrlen, fd, n;
	struct sockaddr_in addr;
	RingManager * ringmanager;
	char buffer[400];
	
	ringmanager = (RingManager*)malloc(sizeof(RingManager));
	ringmanager->succi = NULL;
	ringmanager->predi = NULL;
	ringmanager->id    = 0;		/*INITIALIZATION VALUES PLZ CHANGE*/
	ringmanager->ring  = 0;		/*TEST ONLY*/
	
	return ringmanager;
}

int RingManagerReq(RingManager * ringmanager, fd_set * rfds, Request * request){
	
  char buffer[128];
  
	int n = 0;
	
	if(ringmanager->predi!=NULL && FD_ISSET(ringmanager->predi->fd,rfds)){
		if((n=read(ringmanager->predi->fd,buffer,128))!=0){
			if(n==-1) exit(1);				/*ERROR HANDLING PLZ DO SMTHG EVENTUALLY*/
			buffer[n] = '\0';
      
			RequestParseString(request, buffer);
			
			return 1;
		}
	}
	
	if(ringmanager->succi!=NULL && FD_ISSET(ringmanager->succi->fd,rfds)){
		if((n=read(ringmanager->succi->fd,buffer,128))!=0){
			if(n==-1)exit(1);				/*ERROR HANDLING PLZ DO SMTHG EVENTUALLY*/
			buffer[n] = '\0';
      
      /* Check if request is completely in buffer! (could happen that he only receives half \n */
			
      RequestParseString(request, buffer);
			
      return 1;
		}
	}
	
	return 0;
}
