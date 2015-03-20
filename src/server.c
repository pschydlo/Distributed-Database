#include "server.h"

#define max(A,B) ((A)>=(B)?(A):(B))

struct Server{
	int i;
	RingManager * ringmanager;
	TCPManager * tcpmanager;
	int port;
};

Server * ServerInit(){
	Server * server=(Server*)malloc(sizeof(Server));
	
	server->ringmanager = RingManagerInit();
	server->tcpmanager  = TCPManagerCreate(9001);
	
	return server;
}

int ServerConfigPort(Server * server, int port){
	server->port = port;
	return 1;
}

int ServerStart(Server * server){
	
	int shutdown = 0;
	int argc;
	char buffer[128];
	char args[8][128];
	int maxfd, counter;
	fd_set rfds;
	int n = 0;
	Request *request = RequestCreate();
	
	int fd = TCPSocketCreate();
	TCPSocketBind(fd, server->port);
	if(TCPSocketListen(fd)==-1)exit(1);
	
	while(!shutdown){
		
		FD_ZERO(&rfds); maxfd=0;
		
		FD_SET(fd, &rfds);
		maxfd = (fd > maxfd ? fd : maxfd);
		
		UImanagerArm(&rfds,&maxfd);
		RingManagerArm(server->ringmanager,&rfds,&maxfd);
		TCPManagerArm(server->tcpmanager, &rfds, &maxfd);
		
		counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval*)NULL);
		if(counter<0)exit(1);
		
		if(counter==0)continue;

		n = RingManagerReq(server->ringmanager, &rfds, buffer);
		ServerProcRingReq(server, buffer, n);
		
		n = TCPManagerReq(server->tcpmanager, &rfds, request);
		ServerProcTCPReq(server, request);
		
		n = UImanagerReq(&rfds, request);
		ServerProcUIReq(server, request);
		
		if(FD_ISSET(fd, &rfds)){
			int newfd = TCPSocketAccept(fd);
			RingManagerNew(server->ringmanager, newfd, 560560565);
		}
	}
}

int ServerStop(Server * server){
	free(server);
	return 0;
}

int ServerProcRingReq(Server * server, char * buffer, int n){
	if(n <= 0) return n;

	return 1;
}

int ServerProcTCPReq(Server * server, Request * request){

	return 1;
}

int ServerProcUIReq(Server * server, Request * request){
	
	/*if(n <= 0) return n;*/
	printf("You wrote: %s", RequestGetBuffer(request));
	
	return 1;
}
