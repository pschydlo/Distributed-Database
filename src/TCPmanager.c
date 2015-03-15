#include "TCPmanager.h"

struct Request{
	int fd;
	char * buffer;
};

struct Peer{
	int id, fd;
	char * ip;
};

struct TCPmanager{
	Peer * succi;
	Peer * predi;
	int * fds;
	int fd;
	struct sockaddr_in addr;
	
};

int TCPmanagerRes(TCPmanager * tcpmanager, char * buffer, int nbytes){
	
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
	

TCPmanager * TCPmanagerInit(){
	
	int addrlen, fd;
	struct sockaddr_in addr;
	TCPmanager * tcpmanager;
	
	tcpmanager = (TCPmanager*)malloc(sizeof(TCPmanager));
	tcpmanager->succi = NULL;
	tcpmanager->predi = NULL;
	
	if((fd=socket(AF_INET,SOCK_STREAM,0))==-1)exit(1); /*ERROR HANDLING TO BE DONE PL0X*/
	
	memset((void*)&addr,(int)'\0',sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(9000);
	
	if(bind(fd,(struct sockaddr*)&addr,sizeof(addr))==-1)exit(1); /*ERROR HANDLING TO BE DONE PL0X*/
	printf("1:%i\n",fd);
	
	if(listen(fd,10)==-1)exit(1);
	
	tcpmanager->fd = fd;
	tcpmanager->addr = addr;
	
	return tcpmanager;
}

void TCPmanagerReq(TCPmanager * tcpmanager, char * buffer){

	int newfd, n, addrlen;
	int fd;
	/*fd_set rfds;*/
	struct sockaddr_in addr;
	
	fd = tcpmanager->fd;
	addrlen=sizeof(tcpmanager->addr);
	
	/*int a, fd, maxfd, afd, counter;
	fd_set rfds;
	
	FD_ZERO(&rfds);
	FD_SET(fd,&rfds);
	maxfd = max(maxfd,afd);
	addrlen=sizeof(addr);*/
	
	/*counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval*)NULL);
	if(counter<0)exit(1);
	
	if(FD_ISSET(fd,&rfds)){*/
	if((newfd=accept(fd,(struct sockaddr*)&(tcpmanager->addr),&addrlen))==-1)exit(1);
	printf("2:%i\n",newfd);
	printf("3:%i\n",fd);
	/*}
	
	if(FD_ISSET(afd,&rfds)){*/
		if((n=read(newfd,buffer,128))!=0){
			if(n==-1)exit(1);
			write(newfd,buffer,n);
		}/*else{
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
