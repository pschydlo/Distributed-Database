#include "server.h"

#define max(A,B) ((A)>=(B)?(A):(B))

struct Server{
	int i;
	TCPmanager * tcpmanager;
};

Server * ServerInit(){
	Server * server=(Server*)malloc(sizeof(Server));
	server->tcpmanager=TCPmanagerInit();
	return server;
}

int ServerStart(Server * server){
	
	int shutdown = 0;
	char buffer[128];
	
	while(!shutdown){

		TCPmanagerReq(server->tcpmanager,buffer);
		
		UImanagerReq(buffer);

	}
}

int ServerStop(Server * server){
	free(server);
	return 0;
}
