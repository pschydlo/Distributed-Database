#include "TCPManager.h"

struct TCPManager{
	int pfd;
	int sockets[10];
	/*Buffer *buffer;*/
};

TCPManager * TCPManagerCreate(int port){
	TCPManager * tcpmanager = (TCPManager*)malloc(sizeof(TCPManager));
	
	int pfd = TCPSocketCreate();
	TCPSocketBind(pfd, port);
	if(TCPSocketListen(pfd)==-1) exit(1);
	
	tcpmanager->pfd = pfd;
	tcpmanager->sockets[0] = 0;
	//tcpmanager->buffer = (Buffer*)malloc(sizeof(Buffer));

	return tcpmanager;
}

int TCPManagerArm( TCPManager * tcpmanager, fd_set * rfds, int * maxfd ){

	FD_SET(tcpmanager->pfd, rfds);
	if(tcpmanager->pfd > *maxfd) *maxfd = tcpmanager->pfd;
	
	if(tcpmanager->sockets[0] != 0){
		FD_SET(tcpmanager->sockets[0], rfds);
		if(tcpmanager->sockets[0] > *maxfd) *maxfd = tcpmanager->sockets[0];
	}
	
	return 0;
}

int TCPManagerReq(TCPManager * tcpmanager, fd_set * rfds, Request * request){

	int n = 0;
	
	char buffer[128];
	
	if(FD_ISSET(tcpmanager->pfd,rfds)){
		int newfd = TCPSocketAccept(tcpmanager->pfd);
		tcpmanager->sockets[0] = newfd;
	}
	
	if(tcpmanager->sockets[0]!=0 && FD_ISSET(tcpmanager->sockets[0],rfds)){ /*DONT FORGER ZERO = MINUS ONE #### 0 = -1*/
		if((n=read(tcpmanager->sockets[0],buffer,128))!=0){
			if(n==-1)exit(1);				/*ERROR HANDLING PLZ DO SMTHG EVENTUALLY*/
			buffer[n]='\0';
			printf("External wrote: %s",buffer);
			return 1;
		}
	}

	return 1;
}
