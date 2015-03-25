#include "TCPManager.h"

#define MAX_CON 2

struct TCPManager{
	int pfd, idfd;
	int searchid;
	int sockets[MAX_CON];
	/*Buffer *buffer;*/
};

int TCPManagerIDfd(TCPManager * tcpmanager){
    return tcpmanager->idfd;
}

TCPManager * TCPManagerInit(){
	int i = 0;
	TCPManager * tcpmanager = (TCPManager*)malloc(sizeof(TCPManager));
	
	for(i=0; i < MAX_CON; i++){
		tcpmanager->sockets[i] = -1;
	}
	
	return tcpmanager;
}

int TCPManagerStart(TCPManager * tcpmanager, int TCPport){
	int pfd = TCPSocketCreate();
	TCPSocketBind(pfd, TCPport);
	if(TCPSocketListen(pfd)==-1) exit(1);
	
	tcpmanager->pfd = pfd;
	
	//tcpmanager->buffer = (Buffer*)malloc(sizeof(Buffer));

	return 0;
}

int TCPManagerArm( TCPManager * tcpmanager, fd_set * rfds, int * maxfd ){

	FD_SET(tcpmanager->pfd, rfds);
  //if(tcpmanager->idfd != -1) FD_SET(tcpmanager->idfd, rfds);      /*temp id socket for outgoing ID asking*/
	if(tcpmanager->pfd > *maxfd) *maxfd = tcpmanager->pfd;
	
	int i = 0;
	
	for(i=0; i < MAX_CON; i++){
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
		
		for(i=0; i<MAX_CON; i++){
			if(tcpmanager->sockets[i] != -1) continue;
			
			tcpmanager->sockets[i] = newfd;
			break;
		}
		
		if(i == MAX_CON){
			write(newfd, "Busy try again later.\n", 22);
			close(newfd);
		}
	}
        
       /* if(FD_ISSET(tcpmanager->idfd,rfds)){
            FD_CLR(tcpmanager->idfd, rfds);
            if((n=read(tcpmanager->idfd,buffer,128))!=0){
                if(n==-1)exit(1);
                buffer[n]='\0';
                
                RequestParseString(request, buffer);
                RequestAddFD(request, tcpmanager->idfd);
                
                reqcount = 1;
            }
        }*/
                
                        

	for(i = 0; i < MAX_CON; i++){
		if(tcpmanager->sockets[i] == -1 || !FD_ISSET(tcpmanager->sockets[i],rfds)) continue;
		FD_CLR(tcpmanager->sockets[i], rfds);
		
		n = send(tcpmanager->sockets[i], " ", 1, MSG_NOSIGNAL);
		if (n == -1)
		{
			close(tcpmanager->sockets[i]);
			tcpmanager->sockets[i] = -1;
		
			continue;
		}
		
		if(reqcount == 1) continue;
		
		if((n=read(tcpmanager->sockets[i],buffer,128))!=0){
			if(n==-1)exit(1);				/*ERROR HANDLING PLZ DO SMTHG EVENTUALLY*/
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

int TCPManagerRespond(Request * request, int fd){

	
	return 1;
}
