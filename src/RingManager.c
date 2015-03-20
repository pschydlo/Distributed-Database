#include "RingManager.h"


struct Peer{
	int id, fd;
	char * ip;
};

struct RingManager{
	Peer * succi;
	Peer * predi;
};

int RingManagerNew(RingManager * ringmanager, int fd, int port){
	if(ringmanager->predi == NULL){
		ringmanager->predi = (Peer*)malloc(sizeof(Peer));
		ringmanager->predi->fd = fd;
	}else{
		write(ringmanager->predi->fd, "CON i i.IP i.TCP\n", 17);
		close(ringmanager->predi->fd);
		ringmanager->predi->fd = fd;
	}
}

int RingManagerConnect(RingManager * ringmanager, char * ip, int port){
	
	int n, fd = TCPSocketCreate();

	if((n = TCPSocketConnect(fd, ip, port)) < 0) {
		printf("Could not connect to predi.");
		exit(1);
	} /*ERRORORORORORO! checking to be done*/
	
	write(fd, "NEW 12121 12", 10);
	write(fd, "NEW 12121 12", 10);

	if(ringmanager->succi == NULL) ringmanager->succi = malloc(sizeof(Peer));
	ringmanager->succi->fd = fd;
	
	return n;
}

int RingManagerArm( RingManager * ringmanager, fd_set * rfds, int * maxfd ){
	
	int n=0;
	
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
	
	return ringmanager;
}

int RingManagerReq(RingManager * ringmanager,fd_set * rfds, char * buffer){
	
	int n = 0;
	
	if(ringmanager->predi!=NULL && FD_ISSET(ringmanager->predi->fd,rfds)){
		if((n=read(ringmanager->predi->fd,buffer,128))!=0){
			if(n==-1)exit(1);				/*ERROR HANDLING PLZ DO SMTHG EVENTUALLY*/
			buffer[n]='\0';
			printf("Predi wrote: %s",buffer);
			fflush(stdout);
			write(ringmanager->predi->fd, buffer, 6);
			return 1;
		}
	}
	
	if(ringmanager->succi!=NULL && FD_ISSET(ringmanager->succi->fd,rfds)){
		if((n=read(ringmanager->succi->fd,buffer,128))!=0){
			if(n==-1)exit(1);				/*ERROR HANDLING PLZ DO SMTHG EVENTUALLY*/
			buffer[n]='\0';
			printf("Succi wrote: %s",buffer);
			fflush(stdout);
			return 1;
		}
	}
	
	return 0;
}
