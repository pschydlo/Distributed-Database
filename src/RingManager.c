#include "RingManager.h"

struct Request{
	int fd;
	char * buffer;
};

struct Peer{
	int id, fd;
	char * ip;
};

struct RingManager{
	Peer * succi;
	Peer * predi;
};

int RingManagerNew( RingManager * ringmanager, int fd, int port ){
	
	if( ringmanager->predi == NULL ){
		ringmanager->predi = (Peer*)malloc(sizeof(Peer));
		ringmanager->predi->fd = fd;
	}else{
		write(ringmanager->predi->fd, "CON i i.IP i.TCP\n", 17);	
		close(ringmanager->predi->fd);
		ringmanager->predi->fd = fd;
	}
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
	
	int addrlen, fd;
	struct sockaddr_in addr;
	RingManager * ringmanager;
	
	ringmanager = (RingManager*)malloc(sizeof(RingManager));
	ringmanager->succi = NULL;
	ringmanager->predi = NULL;
	
	/*if((fd=socket(AF_INET,SOCK_STREAM,0))==-1)exit(1); ERROR HANDLING TO BE DONE PL0X
	
	memset((void*)&addr,(int)'\0',sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(9000);
	
	if(bind(fd,(struct sockaddr*)&addr,sizeof(addr))==-1){
		puts("Derp port");
		exit(1); ERROR HANDLING TO BE DONE PL0X
	}
	printf("RingManagerInit, fd=%i\n",fd);
	
	if(listen(fd,10)==-1)exit(1);
	
	ringmanager->fd = fd;
	ringmanager->addr = addr;*/
	
	return ringmanager;
}

void RingManagerReq(RingManager * ringmanager,fd_set * rfds, char * buffer){
	
	int n = 0;
	
	if(ringmanager->succi!=NULL && FD_ISSET(ringmanager->predi->fd,rfds)){
		if((n=read(0,buffer,128))!=0){
			if(n==-1)exit(1);				/*ERROR HANDLING PLZ DO SMTHG EVENTUALLY*/
			buffer[n]='\0';
			printf("Predi wrote: %s",buffer);
		}
	}
	
	if(ringmanager->succi!=NULL && FD_ISSET(ringmanager->succi->fd,rfds)){
		if((n=read(0,buffer,128))!=0){
			if(n==-1)exit(1);				/*ERROR HANDLING PLZ DO SMTHG EVENTUALLY*/
			buffer[n]='\0';
			printf("Succi wrote: %s",buffer);
		}
	}
	
	/*if(FD_ISSET(afd,&rfds)){
		if((n=read(newfd,buffer,128))!=0){
			if(n==-1)exit(1);
			write(newfd,buffer,n);
		}else{
			close(afd);
			state=idle;
		}
	}*/
	
	/*if(FD_ISSET(afd,&rfds)){
		if((n=read(afd,buffer,128))!=0){
			if(n==-1)exit(1);
			write(afd,buffer,n);
		}
	}*/

}
